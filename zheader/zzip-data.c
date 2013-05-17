#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "zzip-data.h"
#include "zlib.h"
#include "zheader/zheader.h"
#include "blowfish/blowfish.h"

char *ZzipData(char *image_type, char *data, size_t *length, char **extra_headers)
{
    // zzip the data without the compression:
    unsigned int rounded_length = *length;
    char *buffer = malloc(*length+1024);   if (!buffer)                  return NULL;
    char *data2 = malloc(*length+8);     if (!data2) { free(buffer); return NULL; }
    char *key = getenv("ZZIP_KEY");
    char key_index = -1;
    uint32_t plaintextCRC;
    uint32_t plaintextChecksum;
    if (key) {
        key_index = strtoul(key, NULL, 10);
        key += 3;
        plaintextCRC = crc32(0, (Bytef*)data, *length);
        // Blowfish (at least our implementation) requires the input to be on an 8 byte
        // boundary. Otherwise when we decrypt we will get garbled data on the last little bit.
        if (rounded_length % 8 != 0)
            rounded_length += 8 - rounded_length % 8;
        memset(data2, 0, rounded_length);
        memcpy(data2, data, *length);
        plaintextChecksum = Checksum((uint8_t*)data2,rounded_length);
        data = data2;
        BlowfishContext context;
        Blowfish_Init(&context,(uint8_t*)key,strlen(key));
        Blowfish_Encrypt_Buffer(&context, (unsigned long*)data, rounded_length);
    }
    int o = 0;
    o += sprintf(buffer+o,"zzip version 1.0 (1394 %s)\n","1.0");
    o += sprintf(buffer+o,"Compressed Length   = 0x%08X\n",rounded_length); // should really be called "encrypted length" (only on encryption) because it's rounded up.
    o += sprintf(buffer+o,"Uncompressed Length = 0x%08X\n",*length);
    o += sprintf(buffer+o,"Checksum            = 0x%08lX\n",Checksum((uint8_t*)data,rounded_length));
    o += sprintf(buffer+o,"CRC                 = 0x%08lX\n",crc32(0, (uint8_t*)data, rounded_length));
    o += sprintf(buffer+o,"Compression         = none\n");
    o += sprintf(buffer+o,"Image Type          = %s\n", image_type);
    if (key) {
        o += sprintf(buffer+o,"Unencrypted Length  = 0x%08X\n",*length); // Should really be called "compressed length" because it's not rounded
        o += sprintf(buffer+o,"Encryption          = blowfish\n");
        o += sprintf(buffer+o,"Encryption Key      = %d\n",key_index);
        o += sprintf(buffer+o,"Plaintext Checksum  = 0x%08X\n",plaintextChecksum);
        o += sprintf(buffer+o,"Plaintext CRC       = 0x%08X\n",plaintextCRC);
    }
    while(extra_headers && *extra_headers)
        o += sprintf(buffer+o, "%s\n", *extra_headers++);
    o += sprintf(buffer+o,"Name                = <on-the-fly>\n");
    time_t t = time(&t);
    o += sprintf(buffer+o,"Date                = %s",ctime(&t));
    memcpy(buffer+o+1, data, rounded_length);
    // Boy that was tough.
    free(data2);
    *length = o+1+rounded_length;
    return buffer;
}
