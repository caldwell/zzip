/* $Header: /cvs/host/zzip/zheader/zheader.h,v 1.14 2004/08/27 02:42:12 david Exp $
-------=====================<<<< COPYRIGHT >>>>========================-------
         Copyright (c) 1995-1998 Indigita Corp,  All Rights Reserved.
 See full text of copyright notice and limitations of use in file COPYRIGHT.h
-------================================================================-------
*/
#include "COPYRIGHT.h"

/**@file
 *
 * @brief This file contains functions that can parse the header of
 * Indigita's zzip format files.
 *
 * @see zheader
 */

/**@defgroup zheader The zzip file format
 * @{
 * The zzip format is a simple format designed to go with various
 * small configuration files or ROM images. It consists of a null
 * terminated string prepended to the actual file data (called the
 * <em>"payload"</em>). The string is constructed in a specific format
 * designed for easy parsing and maximum flexibility. This header is
 * called a <em>"zheader"</em>.
 *
 * The zheader is a series of lines with each line terminated by
 * CRLF, CR, or LF (very flexible).
 *
 * The zheader must start with the line:
 * <br><b><dfn>zzip version <em>number</em> <em>somestring</em></dfn></b><br>
 * where <em>number</em> is the version of the format (currently 1.0)
 * and where <em>somestring</em> is some arbitrary text
 * inserted by the program that created the file.
 *
 * The rest of the zheader lines are of the form:
 * <br><b><dfn>key = value</dfn></b><br>
 * where <em>key</em> can contain any printable ASCII
 * character except "=". There may be zero or more spaces around the
 * the "=" that separates the name from the value. <em>value</em>
 * can contain any printable ASCII charater (including "=").
 *
 * There are several headers that are automatically put in the zheader
 * by "zzip" (which is the only thing that creates zheaders at the
 * moment). Here are a list of them:
 *   - <b>Compressed Length</b>: The length of the payload.
 *   - <b>Uncompressed Length</b>: The length of the payload when
 *     uncompressed. (This will be the
 *     same as <b>Compressed Length</b> if the payload is not compressed.
 *   - <b>Checksum</b>: Calculated by adding the all the bytes of the
 *     payload.
 *   - <b>Compression</b>: The compression type used on the payload.
 *     This is one of:
 *     - none: no compression
 *     - zlib: compressed with the zlib library.
 *     .
 *   - <b>Encryption</b>: The encryption type used on the payload.
 *     This is one of:
 *     - none: no encryption
 *     - blowfish: the payload is encrypted with the blowfish algorithm.
 *     .
 *   - <b>Encryption Key</b>: This will only exist if the <b>Encryption</b>
 *     header is not "none". It will be a integer representing an index into
 *     the keyfile that was used when encrypting the file.
 *   - <b>Name</b>: The name of the file that is now the payload.
 *   - <b>Date</b>: The date the zzip file was created (in unix date string
 *     format).
 *   .
 * The following zheaders are not created by zzip, but are very common:
 *   - <b>Image Type</b>: This specifies what kind of data the payload
 *     contains. <b>NOTE:</b> Even though this is not created by default,
 *     Indigita policy specifies that all zzip files should contain an
 *     <b>Image Type</b>. Also every project that has incompatible zzip
 *     data formats should have a different <b>Image Type</b> header,
 *     and all programs that use zzip files should check to make sure the
 *     <b>Image Type</b> is the correct type.
 *     This should ensure that it is impossible to use the incorrect zzip
 *     file with a program and destroy something (flashing the 8051 bridge
 *     firmware to the AvLink chip would be bad!).
 *   - <b>Load Address</b>: Firmware files have a <b>Load Address</b> so
 *     that the loader knows where to put the payload.
 *   - <b>Start Address</b>: Firmware files have a <b>Start Address</b>
 *     so that the loader knows where to jump to after loading the payload.
 *   .
 * Here is an example zheader:<pre>
 * zzip version 1.0 (1.0)
 * Compressed Length   = 0x00019E30
 * Uncompressed Length = 0x00019E30
 * Checksum            = 0x00CD711F
 * Compression         = none
 * Encryption          = blowfish
 * Encryption Key      = 0
 * Name                = kerneltest.bin
 * Date                = Fri Mar 22 16:12:24 2002
 * Image Type = Firmware
 * Load Address = 0xc0000000
 * Start Address = 0xc0000100
 * </pre>
 * @see zheader.h
 * @}
 */
#ifndef ZHEADER_H
#define ZHEADER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Validates a zheader.
 * This currently checks to make sure the zheader starts with "zzip".
 * @param header a pointer to the zheader.
 * @return <b>true</b> if the zheader is valid. <br>
 *         <b>false</b> if the zheader is <em>not</em> valid.
 */
bool IsValidHeader(char *header);

/**
 * @brief Retrieve an integer from a zheader.
 * This locates a value for the key specified by <em>name</em> in the
 * zheader pointed to by <em>header</em>. If the key is found in the
 * zheader, then the int pointed to by <em>value</em> will be set to
 * the corresponding value of the key found in the zheader, as
 * interpreted by strtoul(). If the key is not found then
 * <em>value</em> will not be touched. This allows you to put a
 * default value in <em>*value</em> and then ignore the return value
 * of this function, if you so choose.
 *
 * @param header a pointer to the zheader.
 * @param name   the key to look for.
 * @param value  a pointer to an integer where the value will be put (if found).
 *
 * @note @e value can be safely NULL. This allows you to check for
 * the existance of a particular key in a zheader.
 *
 * @return <b>true</b> if the key was found in the zheader.<BR>
 *         <b>false</b> if the key was <em>not</em> found in the zheader.
 */
bool GetHeaderNumber(char *header,char *name, unsigned long *value);

/**
 * @brief Retrieve a string from a zheader.
 *
 * This locates a value for the key specified by <em>name</em> in the
 * zheader pointed to by <em>header</em>. If the key is found in the
 * zheader, then the corresponding value of the key found in the
 * zheader will be copied into the space pointed to by
 * <em>value</em>. If the key is not found then <em>value</em> will
 * not be touched. This allows you to put a default string in
 * <em>value</em> and then ignore the return value of this function,
 * if you so choose.
 *
 * @param header a pointer to the zheader.
 *
 * @param name   the key to look for.
 *
 * @param value a pointer to a string where the value will be put (if
 * found).<b>Note that this function does not allocate any space for
 * <em>value</em></b>.
 *
 * @param length the length that the calling function has allocated
 * for <em>value</em>. GetHeaderString will only write the first
 * <em>length</em> bytes of the header string into
 * <em>value</em>. <em>value</em> will always be properly NULL
 * terminated.
 *
 * @note @e value can be safely NULL. This allows you to check for
 * the existance of a particular key in a zheader.
 *
 * @return <b>true</b> if the key was found in the zheader.<BR>
 *         <b>false</b> if the key was <em>not</em> found in the zheader.
 */
bool GetHeaderString(char *header,char *name, char *value, int length);

/**
 * @brief Retrieve a boolean value
 *
 * This locates a value for the key specified by <em>name</em> in the
 * zheader pointed to by <em>header</em>. If the key is found in the
 * zheader and it is one of <b>true</b>,<b>1</b>,<b>yes</b> or
 * <b>y</b> then <em>value</em> is set to <b>true</b>, if it is one of
 * <b>false</b>,<b>0</b>,<b>no</b> or <b>n</b> then <em>value</em> is
 * set to <b>false</b>, otherwise <em>value</em> is unchanged and
 * <b>false</b> is returned.
 *
 * @param header a pointer to the zheader.
 *
 * @param name   the key to look for.
 *
 * @param value  a pointer to an bool where the value will be put (if found).
 *
 * @note @e value can be safely NULL. This allows you to check for
 * the existance of a particular key in a zheader.
 *
 * @return <b>true</b> if the key was found in the zheader.<BR>
 *         <b>false</b> if the key was <em>not</em> found in the zheader.
 */
bool GetHeaderBoolean(char *header, char *name, bool *value);

/**
 * @brief Returns the length of the zheader.
 * This currently is implemented by just calling strlen(zheader).
 *
 * @param header a pointer to the zheader.
 * @return the length of the zheader (in bytes).
 */
unsigned long HeaderLength(char *header);

/**
 * @brief Calculates a checksum for some data.
 * This calculates a checksum by adding all <em>l</em> bytes in
 * the memory pointed to by <em>c</em>.
 *
 * @param c pointer to some data.
 * @param l length of the data.
 * @return the calculated checksum.
 */
unsigned long Checksum(unsigned char *c,unsigned long l);

/**
 * @brief Find the value for a key in the zheader.
 * This will find the pointer to a value in the zheader.
 *
 * @param header a pointer to the zheader.
 * @param name   the key to look for.
 *
 * @return the pointer to the point in the zheader where the value
 *         starts.<br>
 *         <b>NULL</b> if the key couldn't be found.
 */
char * LocateName(char *header,char *name);

/**
 * @brief Sucks up whitespace.
 * Skips over space, tab, CR and LFs.
 *
 * @param in input string.
 * @return the point at which the white space ends.
 */
char * EatSpace(char *in);

/**
 * @brief Iterate through the keys of a zheader.
 * Returns the next non null terminated key which
 * has length <em>length</em>.
 *
 * @param name previous input string or the zheader
 *        itself on the first call.
 * @param length is set to the length of the string.
 * @return the next non null terminated key which
 *         has length <em>length</em>.  returns NULL
 *         when done.
 */
char *NextHeaderName(char *name, int *length);

#if 0
int NumberOfImagesInBundle(char *firstHeader, int maxLength);
char * GetImageFromBundle(char *firstHeader, int number);
#endif

#ifdef __cplusplus
}
#endif

#endif

/*
 * $Log: zheader.h,v $
 * Revision 1.14  2004/08/27 02:42:12  david
 * - Printing the header is easy enough... No need for a function.
 *
 * Revision 1.13  2004/05/20 23:20:47  david
 * - Put zheader documentation into its own section.
 *
 * Revision 1.12  2003/10/13 23:28:02  radford
 * o Make the zheader and database interfaces clearer by using bools.
 *
 * Revision 1.11  2003/10/06 20:39:24  neal
 * add or change doxygen comments for documentation.
 *
 * Revision 1.10  2003/07/29 01:04:57  radford
 * o Added GetHeaderBoolean with lots of different bool representations
 *   at David's insistence.
 *
 * Revision 1.9  2003/06/27 23:09:50  radford
 * o Added command line completion for files, preferences and commands.
 *
 * Revision 1.8  2003/03/24 13:45:29  radford
 * o GetHeaderString now takes the length its buffer. This squashes a
 *   few buffer overflows in the rom-loader/loader/firmware.
 *
 * Revision 1.7  2002/04/20 00:41:27  david
 * - Made EatSpace() and LocateName() non-static so the preferences code
 *   can use them.
 *
 * Revision 1.6  2002/03/30 06:52:04  david
 * - Allow passing the passing in of NULL to GetHeaderNumber() and
 *   GetHeaderString(). This can be used to check for the existance
 *   of a particular key in the header without getting it.
 *
 * Revision 1.5  2002/03/28 01:22:46  david
 * - I accidentally left in a stray test declaration.
 *
 * Revision 1.4  2002/03/26 20:11:50  david
 * - The log keyword was missing from this file for some reason.
 *
 *
 * *****************  Version 6  *****************
 * User: Roy          Date: 2/16/99    Time: 1:48p
 * Updated in $/Firmware/Forehead/Loader
 * o  Killed old style header.
 *
 * *****************  Version 5  *****************
 * User: Radford      Date: 7/27/98    Time: 7:10p
 * Updated in $/Firmware/Loader
 * David: Added checksum routine (was moved from zzip.c).
 *
 * *****************  Version 4  *****************
 * User: David        Date: 10/22/97   Time: 1:20a
 * Updated in $/Sunflower/Loader/SFF
 * I wrote these great functions, then realized I couldn't use them with
 * our loaders stupid scatter-gather implementation. Maybe someone else
 * can use them.
 *
 * *****************  Version 3  *****************
 * User: David        Date: 1/21/97    Time: 5:29p
 * Updated in $/Sunflower/Loader/SFF
 * added endif
 *
 * *****************  Version 2  *****************
 * User: David        Date: 1/21/97    Time: 5:26p
 * Updated in $/Sunflower/Loader/zzip
 * Capitalized h
 */
