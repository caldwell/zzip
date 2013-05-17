/* $Header: /cvs/host/zzip/error/werror.c,v 1.1 2003/11/18 01:50:08 radford Exp $
-------=====================<<<< COPYRIGHT >>>>========================-------
         Copyright (c) 2003 Indigita Corp,  All Rights Reserved.
 See full text of copyright notice and limitations of use in file COPYRIGHT.h
-------================================================================-------
*/
#include "COPYRIGHT.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "werror.h"

#include <windows.h>
#include <winbase.h>


void werror(int error, int status, char *format, ...)
{
    va_list ap;
    va_start(ap,format);
    fprintf(stderr, "error: ");
    vfprintf(stderr, format, ap);
    if (status) {
        char message[1000];
        if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, status, 0, message, sizeof(message), NULL))
            fprintf(stderr, ": %s", message);
        else
            fprintf(stderr, ": Things suck so bad (%d) I can't even get a message for you.", status);
    }
    fprintf(stderr,"\n");
    va_end(ap);
    if (error)
        exit(error);
}

/*
 * $Log: werror.c,v $
 * Revision 1.1  2003/11/18 01:50:08  radford
 * o On windows you can pass GetLastErorr() to werror() instead of error()
 *   to print out system messages.
 *
 */
