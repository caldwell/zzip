/* $Header: /cvs/host/zzip/error/error.c,v 1.3 2003/11/11 00:46:57 radford Exp $
-------=====================<<<< COPYRIGHT >>>>========================-------
         Copyright (c) 2003 Indigita Corp,  All Rights Reserved.
 See full text of copyright notice and limitations of use in file COPYRIGHT.h
-------================================================================-------
*/
#include "COPYRIGHT.h"

#include <error.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef __GLIBC__
void error(int error, int errno_, char *format, ...)
{
    va_list ap;
    va_start(ap,format);
    fprintf(stderr, "error: ");
    vfprintf(stderr, format, ap);
    if (errno_)
        fprintf(stderr, ": %s", strerror(errno_));
    fprintf(stderr,"\n");
    va_end(ap);
    if (error)
        exit(error);
}
#endif

/*
 * $Log: error.c,v $
 * Revision 1.3  2003/11/11 00:46:57  radford
 * o Avoid conflict with the global errno in the prototype or error().
 * o We still should print when errno is zero.
 *
 * Revision 1.2  2003/09/10 20:39:02  radford
 * o Actually implement the real semantics.
 *
 * Revision 1.1  2003/09/10 18:56:29  radford
 * o A simple version of glibc's error().
 *
 */
