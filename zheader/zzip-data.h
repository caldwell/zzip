/* $Header: /cvs/host/zzip/zheader/zzip-data.h,v 1.3 2003/09/23 20:32:35 neal Exp $
-------=====================<<<< COPYRIGHT >>>>========================-------
         Copyright (c) 2003 Indigita Corp,  All Rights Reserved.
 See full text of copyright notice and limitations of use in file COPYRIGHT.h
-------================================================================-------
*/
#include "COPYRIGHT.h"

#ifndef __ZZIP_DATA_H__
#define __ZZIP_DATA_H__

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @file zzip-data.h
 * @brief builds a zzip output file (in memory)
 */

#include <stdlib.h>

/**
 * @brief routine to build a zzip output file.
 *
 * @param image_type script or preference or ...
 * @param data data to zip up
 * @param length length of the data to zip
 * @param extra_headers extra headers for the output file
 *
 * @returns Gives back a malloced buffer holding the complete zzipped file
 */
char *ZzipData(char *image_type, char *data, size_t *length, char **extra_headers);

#ifdef __cplusplus
}
#endif

#endif /* __ZZIP_DATA_H__ */

/*
 * $Log: zzip-data.h,v $
 * Revision 1.3  2003/09/23 20:32:35  neal
 * added doxygen tags and moved/added comments for documentation.
 *
 * Revision 1.2  2003/09/13 00:19:11  radford
 * o Allow extra headers to be passed to ZzipData.
 *
 * Revision 1.1  2003/08/13 22:21:14  radford
 * o Abstract shell script generator into zzip-minimal library.
 *
 */
