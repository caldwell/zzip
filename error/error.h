/* $Header: /cvs/host/zzip/error/error.h,v 1.3 2003/11/11 00:46:43 radford Exp $
-------=====================<<<< COPYRIGHT >>>>========================-------
         Copyright (c) 2003 Indigita Corp,  All Rights Reserved.
 See full text of copyright notice and limitations of use in file COPYRIGHT.h
-------================================================================-------
*/
#include "COPYRIGHT.h"

#ifndef __ERROR_H__
#define __ERROR_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"{
#endif

void error(int error, int errno_, char *format, ...);

// David's alternate define version
#if 0
# define error(exitcode, errorcode, ...)                                    \
         do { fprintf(stderr,__VA_ARGS__);                                  \
             if (errorcode) { fprintf(stderr, ": "); strerror(errorcode); } \
             if (exitcode) { exit(exitcode); }                              \
         } while (0)
#endif
    
#ifdef __cplusplus
}
#endif

#endif /* __ERROR_H__ */

/*
 * $Log: error.h,v $
 * Revision 1.3  2003/11/11 00:46:43  radford
 * o Avoid conflict with the global errno in the prototype or error().
 *
 * Revision 1.2  2003/09/10 23:59:28  david
 * - Moved my define into here. Don't know if it's better than the C on
 *   ore not. But here it is anyway.
 *
 * Revision 1.1  2003/09/10 18:56:29  radford
 * o A simple version of glibc's error().
 *
 */
