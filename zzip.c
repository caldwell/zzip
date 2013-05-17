/* $Header: /cvs/host/zzip/zzip.c,v 1.24 2003/11/27 00:37:23 david Exp $
-------=====================<<<< COPYRIGHT >>>>========================-------
         Copyright (c) 2001 Indigita Corp,  All Rights Reserved.
 See full text of copyright notice and limitations of use in file COPYRIGHT.h
-------================================================================-------
*/
#include "COPYRIGHT.h"

#undef LZOLibNotZlib
#ifdef LZOLibNotZlib
#include <lzo1x.h>
#define Extension ".lzo"
#else
#include <zlib.h>
#define Extension ".z"
#endif
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <time.h>
#include "blowfish.h"
#include <sys/mman.h>
#include <stdint.h>
#include <ctype.h>
#include <error.h>
#include <errno.h>

#include "zheader.h"

#define version "2.4"

char *ParseKeyFile(char *file, int index);
void UnzipIt(char *zfile, BlowfishContext *blowfish, char *output);

void Usage(char *me)
{
    fprintf(stderr,"usage: %s [-p | -f | (-F [-L|-S|-M] -d<destination file>)] [-E<key-file] [-e<key>] [-v] [-D] [-Z] [-h<extra-header>] [-o<output file>] <binary-image>\n",me);
    fprintf(stderr,"       %s  [-E<key-file] [-e<key>] [-v] [-s] -u [-o<output file>] <zzip-file>\n",me);
    fprintf(stderr,"      -s Use stupid short DOS filenames.\n"
                   "      -Z Dont compress.\n"
                   "      -v Show Version number\n"
                   "      -h<extra-header> Adds <extra-header> to the zzip header. This must be\n"
                   "                       a complete header, eg. \"key = value\".\n"
                   "      -E<key-file> Use this file to get the encryption key out of.\n"
                   "      -e<key> Encrypt image with key index of <key> (from key file).\n"
                   "      -u Unzip instead of zip. (Implied if the program name contains \"zunzip\").\n"
                   "\n"
                   "      -p preferences\n"
                   "      -f firmware\n"
                   "      -F file upload\n"
                   "         -L large file\n"
                   "         -S small file\n"
                   "         -M edium file\n"
                   "      -d<destination> This is the filename used when the file is uploaded.\n"
                   "      -o<output file> The output is written to this file instead of the default.\n"
                   "                      Use '-' for stdout.\n"
                   "      <zzip-file>     The file to un-zzip.  stdin is used if not specified.\n"
                   "      <binary-image>  The file to zzip.     stdin is used if not specified.\n"
            );
    fprintf(stderr,"\nThis program converts a file to a compressed/encrypted file with\n"
                   "extensible, easily parsed meta-data\n\n"
                   "There are 2 keyfile formats. Version 1:\n"
                   "  The key file is just a list of keys. Keys can be any length.\n"
                   "  Each key is separated by a single null.\n"
                   "Version 2:\n"
                   "  The first line must be exactly \"Version 2 Keyfile:\"\n"
                   "  Each line after that should be \"xx yyyyyyyyyy\" where 'xx' is the index and\n"
                   "  'yyyyy' is the key. The index starts at zero and increments by one for each key.\n"
                   "  The index should be zero padded for indices < 10.\n"
            );
    exit(1);
}

int main(int c,char **v)
{
    int r,e;
    char *extraHeader[c],*filename = NULL;
    long extraHeaders=0;
    FILE *f = NULL;

#ifdef LZOLibNotZlib
    lzo_uint zlen;
    void *workmem;
#else
    unsigned long zlen;
#endif
    unsigned char *zrom, *zfile = NULL;
    struct stat s;
    int StupidDosCantHandleLongFileNames = 0;
    int dontCompress = 0;
    int encryptIndex = -1;
    int encryption = 0;
    char *encryptFile = NULL;
    int plaintextChecksum;
    char *type = 0, *file_type = 0, *destination = 0;
    int unzip = !!strstr(v[0], "zunzip");
    char *output = NULL;

    /* parse command line */
    while (--c) /* we must skip first argument (0) as that is OUR program name, so we pre-decrement c */
        if (v[c][0] == '-')
            if (v[c][1] == 'h') /* extra headers */
                extraHeader[extraHeaders++]=&v[c][2];
            else if (v[c][1] == 's') /* Stupid dos file names*/
                StupidDosCantHandleLongFileNames = 1;
            else if (v[c][1] == 'Z') /* and of course, dont compress */
                dontCompress = 1;
            else if (v[c][1] == 'v') { /* give version info */
                printf("%s version %s\n", v[0],version);
                exit(0);
            }
            else if (v[c][1] == 'e') {/* encrypt! */
                encryption = 1;
                if (v[c][2])
                    encryptIndex = strtoul(&v[c][2],NULL,0);
            } else if (v[c][1] == 'E') /* encryption file */
                encryptFile = &v[c][2];
            else if (v[c][1] == 'u') /* unzip */
                unzip = 1;
            else if (v[c][1] == 'p')
                type = "Gem Preferences";
            else if (v[c][1] == 'f')
                type = "Firmware";
            else if (v[c][1] == 'F')
                type = "File";
            else if (v[c][1] == 'L')
                file_type = "large";
            else if (v[c][1] == 'S')
                file_type = "small";
            else if (v[c][1] == 'M')
                file_type = "medium";
            else if (v[c][1] == 'd')
                destination = &v[c][2];
            else if (v[c][1] == 'o')
                output = &v[c][2];
            else if (v[c][1] == '-')
                break;
            else {
                fprintf(stderr,"'%c' is an invalid option\n",v[c][1]);
                Usage(v[0]);
            }
        else /* no dash must mean it's a filename */
            if (!filename)
                filename = v[c];
            else {
                fprintf(stderr,"Only one file may be specified; can't compress \"%s\" AND \"%s\".\n",filename,v[c]);
                Usage(v[0]);
            }

    /* Setup Encrypt/Decryupt now, if we're supposed to: */
    BlowfishContext context;
    if (encryption) {
        char *key = getenv("ZZIP_KEY");
        if (key) {
            encryptIndex = strtoul(key, NULL, 10);
            key += 3;
            Blowfish_Init(&context,key,strlen(key));
        } else if (encryptIndex >= 0) {
            if (!encryptFile)
                error(EXIT_FAILURE, 0, "Looks like you forgot the -E command line option.");
            key = ParseKeyFile(encryptFile, encryptIndex);
            if (key == NULL)
                exit(1);
            Blowfish_Init(&context,key,strlen(key));
            free(key);
        } else
            error(EXIT_FAILURE, 0, "Looks like you should set ZZIP_KEY.");
    }

    if (encryptFile && !encryption)
        error(EXIT_FAILURE, 0, "option -E also requires option -e.");
    
    if (!filename) {
        r = 0;
    } else
        r = open(filename, O_RDONLY
#ifdef MSDOS
             |O_BINARY
#endif
             );
    if (r==-1)
        error(EXIT_FAILURE, errno, "%s: can't open %s",v[0],filename);

    unsigned char *rom = MAP_FAILED;
    if (fstat(r,&s) != -1)     
        rom = mmap(0, s.st_size, PROT_READ|PROT_WRITE , MAP_PRIVATE, r, 0);

    if (rom == MAP_FAILED) {
        int rom_size = 1024*1024, size = 0, got = 0;
        rom = NULL;
        do {
            if (size <= rom_size)
                if (!(rom = realloc(rom, rom_size *= 2)))
                    error(EXIT_FAILURE, 0, "out of memory");
            got = read(r, rom+size, rom_size-size);
            if (got < 0)
                error(EXIT_FAILURE, errno, "read");
            size += got;
        } while (got);
        
        s.st_size = size; // all we use is st_size
    }

    uint32_t plaintextCRC = crc32(0, rom, s.st_size);

    if (unzip) {
        UnzipIt(rom, encryption ? &context : NULL, output);
        exit(0);
    }

    if (dontCompress) {
        zlen = s.st_size;
        zrom = rom;
    } else {
        zrom  = calloc(1,zlen = s.st_size * 101 / 100 + 8); /* read zlib.h */
        if(zrom ==0)
            error(EXIT_FAILURE, 0, "%s: No memory for zrom length %d",v[0], zlen);
#ifdef LZOLibNotZlib
        workmem=malloc(LZO1X_999_MEM_COMPRESS);
        if(workmem==0)
            error(EXIT_FAILURE, 0, "%s: No memory for workmem length %d",v[0], LZO1X_999_MEM_COMPRESS);
        if((e=lzo1x_999_compress(rom,s.st_size,zrom,&zlen,workmem)) != LZO_E_OK)
            error(EXIT_FAILURE, 0, "%s: compress error %d",v[0],e);
#else
        if((e=compress(zrom,&zlen,rom,s.st_size)) != Z_OK)
            error(EXIT_FAILURE, 0, "%s: compress error %d",v[0],e);
#endif
    }
    close(r);
    
    int zlen_orig = zlen;
    int unencryptedCRC = crc32(0, zrom, zlen);
    /* Encrypt now, if we're supposed to: */
    if (encryption) {
        // Blowfish (at least our implementation) requires the input to be on an 8 byte
        // boundary. Otherwise when we decrypt we will get garbled data on the last little bit.
        plaintextChecksum = Checksum(zrom,zlen_orig);
        if (zlen % 8 != 0)
            zlen += 8 - zlen % 8;
        Blowfish_Encrypt_Buffer(&context, (unsigned long*)zrom, zlen);
    }
    
    if (output && strcmp(output, "-"))
        zfile = output;
    
    if (!output && filename) {
        char *c;
        zfile = malloc(strlen(filename)+3);
        strcpy(zfile,filename);
        if (StupidDosCantHandleLongFileNames)
            if ((c = strrchr(zfile,'.'))) /* cheesy dos filenames will only have one "." */
                *c='\0';
        strcat(zfile, Extension);
    }
    if (zfile) {
        if ((f = fopen(zfile, "wb"))==0)
            error(EXIT_FAILURE, 0, "%s: can't open %s",v[0],zfile);
    } else
        f = stdout;
    
    /* do header */
    fprintf(f,"zzip version 1.0 (%s)\n",version);
    fprintf(f,"Compressed Length   = 0x%08lX\n",zlen); // should really be called "encrypted length" (only on encryption) because it's rounded up.
    fprintf(f,"Uncompressed Length = 0x%08lX\n",s.st_size);
    fprintf(f,"Checksum            = 0x%08lX\n",Checksum(zrom,zlen));
    fprintf(f,"CRC                 = 0x%08lX\n",crc32(0, zrom, zlen));
    if (dontCompress)
        fprintf(f,"Compression         = none\n");
    else
        fprintf(f,"Compression         = %s\n",
#ifdef LZOLibNotZlib
                "lzo"
#else
                "zlib"
#endif
                );
    if (type)
        fprintf(f,"Image Type          = %s\n", type);
    if (file_type)
        fprintf(f,"File Type           = %s\n", file_type);
    if (destination)
        fprintf(f,"Destination Path    = %s\n", destination);
    if (encryption) {
        fprintf(f,"Unencrypted Length  = 0x%08lX\n",zlen_orig); // Should really be called "compressed length" because it's not rounded
        fprintf(f,"Unencrypted CRC     = 0x%08lX\n",unencryptedCRC); // Should really be called "compressed CRC" because it's not rounded
        fprintf(f,"Encryption          = blowfish\n");
        fprintf(f,"Encryption Key      = %d\n",encryptIndex);
        fprintf(f,"Plaintext Checksum  = 0x%08lX\n",plaintextChecksum); // "Unencrypted Checksum" (may still be compressed)
        fprintf(f,"Plaintext CRC       = 0x%08lX\n",plaintextCRC);
    }
    if (filename)
        fprintf(f,"Name                = %s\n",filename);
    fprintf(f,"Date                = %s",ctime(&s.st_mtime));

    while (extraHeaders--)
        fprintf(f,"%s\n",extraHeader[extraHeaders]);

    fputc(0,f); /* zero terminates header portion */

    /* write the rest of the file */
    if (fwrite(zrom,zlen,1,f) == -1)
        error(EXIT_FAILURE, 0, "%s: can't write to %s",v[0],zfile);
    fclose(f);
    exit (0);
}

char *ParseKeyFile(char *file, int index)
{
    struct stat s;
    FILE *f = fopen(file,"r");
    char *buf=0;
    char *key=0;
    int i,j,k;
    
    if (!f) {
        error(0, errno, "Couldn't open %s", file);
        goto error;
    }
    
    if(stat(file,&s) == -1) {
        error(0, errno, "Couldn't stat %s", file);
        goto error;
    }

    buf = malloc(s.st_size);
    if (!buf) {
        fprintf(stderr, "No memory for %s", file);
        goto error;
    }
    
    if(fread(buf,1,s.st_size,f) != s.st_size) {
        error(0, errno, "Couldn't read %s", file);
        goto error;
    }

    if (strncmp(buf, "Version 2 Keyfile", sizeof("Version 2 Keyfile")-1) == 0) {
        char *b;
        int keyNum;
        // Version 2 Key format:
        i=0;
        b = strtok(buf, "\n"); // start it off and skip the first line (the version part).
        while (b = strtok(NULL, "\n\r")) {
            // The line should be formatted like this: "01) Key text blah blah blah"
            if (sscanf(b,"%d)", &keyNum) == 1) {
                i++;
                if (keyNum == index) {
                    while (isdigit(*b))
                        b++;
                    while (*b == ')')
                        b++;
                    while (isspace(*b))
                        b++;
                    key = strdup(b);
//                    fprintf(stderr,"The key selected was \"%s\"\n", key);
                    goto gotkey;
                }
            }
        }
        i--; // Hack. :-)
    } else {
        // Version 1 Key format:
        for (j=0,i=0;j<s.st_size;j++) {
            if (i == index) {
                // This is the key we're looking for.
                // so find where it ends.
                for (k=j;k<s.st_size;k++) {
                    if (buf[k] == '\0')
                        break;
                }
                // Either it ran off the end or we hit the form feed. Either way k holds
                // the end.
                key = malloc(k-j+1);
                if (!key) {
                    fprintf(stderr,"Out of memory!\n");
                    goto error;
                }
                memcpy(key,&buf[j],k-j);
                key[k-j] = '\0'; // make sure theres a NULL for them
                
                goto gotkey;
            }       
            if (buf[j] == '\0')
                i++;
        }
    }
    
    fprintf(stderr,"There is no key %d in \"%s\". There is only 0 through %d!\n",
            index, file, i);

  gotkey:
  error:
    if (f)
        fclose(f);
    if (buf)
        free(buf);
    return key;
}

#define Min(a,b) ((a)<(b)?(a):(b))
// Slower than doing it all in one chunk, but uses less stack and no mallocs
static void DecryptInChunks(char *data, int length, BlowfishContext *blowfish)
{
    long aligned[1024/sizeof(long)];
    while (length) {
        int thisChunk = Min(sizeof(aligned), length);
        memcpy(aligned, data, thisChunk);
        Blowfish_Decrypt_Buffer(blowfish, aligned, thisChunk);
        memcpy(data, aligned, thisChunk);
        data += thisChunk;
        length -= thisChunk;
    }
}

char *GetZHeaderPayload(char *header, char **dataOut, size_t *lengthOut, unsigned long *realLengthOut,
                        BlowfishContext *blowfish)
{
    if (!IsValidHeader(header))
        return "eNoZHeader";
    
    char *data = header + HeaderLength(header);
    unsigned long length;
    
    if (!GetHeaderNumber(header, "Compressed Length", &length))
        return "eNoCompressedLength";
    
    unsigned long checksum=0;
    if (!GetHeaderNumber(header, "Checksum", &checksum))
        return "eNoChecksum";
    
    unsigned long cs = Checksum(data, length);
    if (checksum != cs) {
        fprintf(stderr,"Checksum mismatch: (calculated 0x%x, should be 0x%x)", cs, checksum);
        return "eChecksumMismatch";
    }
    
    // The data was possibly encrypted.... Have to do a couple more things....
    char encryption[64] = "none";
    GetHeaderString(header, "Encryption", encryption, sizeof(encryption));
    int isEncrypted = strcmp(encryption, "blowfish") == 0;

    unsigned long realLength = length;
    if (isEncrypted) {
        if (!blowfish)
            return "eImageIsEncrypted";
        
        if (!GetHeaderNumber(header, "Uncompressed Length", &realLength))
            return "eNoUncompressedLength";

        DecryptInChunks(data, length, blowfish);
        memset(blowfish, 0, sizeof(*blowfish));
    }
    
    // Don't set these until we know everything worked
    *dataOut = data;
    *lengthOut = length;
    *realLengthOut = realLength;
    return NULL;
}

void UnzipIt(char *zfile, BlowfishContext *blowfish, char *output)
{
    char *data;
    size_t length;
    unsigned long uncompressedLength;
    char *status = GetZHeaderPayload(zfile, &data, &length, &uncompressedLength, blowfish);
    if (status)
        error(EXIT_FAILURE, 0, "Couldn't parse zheader: %s", status);
    char *uncompressed = malloc(uncompressedLength);
    if (!uncompressed)
        error(EXIT_FAILURE, 0, "No memory for uncompressed data length %d", uncompressedLength);
    char compression[32] = "none";
    GetHeaderString(zfile, "Compression", compression, sizeof(compression));
    if (strcmp(compression, "zlib") == 0) {
        int err;
        if ((err = uncompress(uncompressed, &uncompressedLength, data, length)) != Z_OK)
            error(EXIT_FAILURE, 0, "Uncompression error %d", err);
    } else if (strcmp(compression, "none") == 0) {
        memcpy(uncompressed, data, uncompressedLength);
    } else
        error(EXIT_FAILURE, 0, "Unknown compression type: %s", compression);

    if (blowfish) {
        unsigned long crc;
        if (!GetHeaderNumber(zfile, "Plaintext CRC", &crc)) {
            fprintf(stderr, "warning: No Plaintext CRC\n");
        } else {
            int thecrc = crc32(0, uncompressed, uncompressedLength);
            if (crc != thecrc)
                error(EXIT_FAILURE, 0, "Plaintext CRC mismatch: (calculated 0x%x, should be 0x%x)", thecrc, crc);
        }
    }
    char *name;
    char name_buf[64];
    if (output)
        name = strdup(output);
    else {
        if (!GetHeaderString(zfile, "Name", name_buf, sizeof(name_buf)))
            error(EXIT_FAILURE, 0, "No \"Name\" in header.");
        name = strdup(name_buf);
    }
    if (!name)
        error(EXIT_FAILURE, 0, "Out of memory for name.");
    FILE *f = fopen(name, "w");
    if (!f)
        error(EXIT_FAILURE, 0, "Couldn't open output file \"%s\".", name);
    fwrite(uncompressed, uncompressedLength, 1, f);
    fclose(f);
    free(name);
    exit(0);
}

/*
 * $Log: zzip.c,v $
 * Revision 1.24  2003/11/27 00:37:23  david
 * - Added unencrypted CRC which is calculated between compression and
 *   encryption. This lets the Gem ROM Loader check the output of
 *   decryption without having to uncompress first.
 * - Upped the version for release.
 *
 * Revision 1.23  2003/09/09 02:56:48  radford
 * o Handle some invalid -e and -E option combinations.
 *
 * Revision 1.22  2003/09/08 23:47:00  radford
 * o replace perror()/exit(1) with error().
 * o Fix a few compile errors and mispellings.
 * o Add in ZZIP_KEY environment support like in 1394.
 *
 * Revision 1.21  2003/09/08 23:23:47  radford
 * o Get the encryption key from the environment on -e with no index.
 *
 * Revision 1.20  2003/09/08 19:55:35  radford
 * o Fix a realloc offset bug
 * o bump up the realloc increment and initial size
 *   now that it is tested.
 *
 * Revision 1.19  2003/09/05 00:58:10  radford
 * o Allow stdin and stdout
 *
 * Revision 1.18  2003/09/02 20:52:47  radford
 * o Explain that the "Plaintext Checksum" is over compressed data when compressing.
 *
 * Revision 1.17  2003/09/02 20:49:23  radford
 * o Stop checksumming random garbage encryption padding
 * o Fix plaintex checksum to be only over the actualy plaintext
 *   not including the encryption padding.  This should be backwards
 *   compatible because of the first change.
 *
 * Revision 1.16  2003/07/28 23:49:36  radford
 * o Fix a destination path typo
 *
 * Revision 1.15  2003/07/28 23:15:02  radford
 * o -l should have been -L
 * o use stderr for all errors
 *
 * Revision 1.14  2003/07/24 04:23:24  david
 * - Updated version to 2.2 for immenent release.
 * - Changed -o option to -d.
 * - Added new -o option to set output filename.
 * - Fixed mmap bug that segfaulted zunzip.
 * - Added CRC32s for both plaintext and encrypted+compressed data.
 * - Made out of memory errors more specific because I was getting one of them.
 * - Moved plaintext checksum back where it was because the Gem ROM
 *   loader expects it a certain way and it's hard-coded in ROM!
 * - Added "Unencrypted length" to get the real, post compressed, pre padded length.
 * - Added unzip support for non-compressed files.
 * - Warn when not unzipping a file with no plaintext crc.
 * - Don't even check for plaintext checksum any more since it's an aberration.
 * - Fixed warnings.
 *
 * Revision 1.13  2003/07/23 01:28:53  radford
 * o Add command switches for preferences, file uploads, and firmware.
 *
 * Revision 1.12  2003/07/23 01:26:50  radford
 * o mmap when needed to support really large files.
 *
 * Revision 1.11  2003/07/19 02:21:12  david
 * - Version 2.1
 *
 * Revision 1.10  2003/07/19 02:17:22  david
 * - Added unzip capability.
 * - Moved plaintext checksum so it happens before compression and uses original file length.
 *
 * Revision 1.9  2003/04/09 22:33:15  david
 * - New version 2.0 to signify non-backwards compatibility with zzip 1.2
 *   (blowfish changed)
 *
 * Revision 1.8  2002/09/18 21:02:07  david
 * - Added help for version 2 keyfile.
 *
 * Revision 1.7  2002/09/18 20:35:00  david
 * - Added support for a new key file format (version 2). It is easier to deal with
 *   than the old format. It still supports the old format.
 * - Upped version for release.
 *
 * Revision 1.6  2002/04/18 02:51:52  david
 * - If we are encrypting, we now align the length of the data to an 8
 *   byte boundary. This happens after compressing, as compressing changes
 *   the length (duh.). This fixes the problem where the loader would get
 *   decompression errors on most files that were both compressed and
 *   encrypted. Some files would be aligned after compression and so they
 *   would work, and that really threw me off.
 *
 * Revision 1.5  2002/03/30 00:43:50  david
 * - It's now version 1.0.
 *
 * Revision 1.4  2001/12/14 03:20:32  david
 * I got rid of the compiled in keys. Instead I make the user select a key
 * file on the command line as well as a index into that key file.
 * This is a much more flexible way of doing things. This way the Key
 * File can reside with the project instead compiled into this program,
 * which was designed to be project agnostic.
 *
 * Revision 1.3  2001/12/14 00:48:14  david
 * - Added standard headers/footers.
 * - M-x untabify
 *
 */
