/* $Header: /cvs/host/zzip/error/werror.h,v 1.1 2003/11/18 01:50:09 radford Exp $
-------=====================<<<< COPYRIGHT >>>>========================-------
         Copyright (c) 2003 Indigita Corp,  All Rights Reserved.
 See full text of copyright notice and limitations of use in file COPYRIGHT.h
-------================================================================-------
*/
#include "COPYRIGHT.h"

#ifndef __WERROR_H__
#define __WERROR_H__

#ifdef __cplusplus
extern "C"{
#endif

void werror(int error, int status, char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* __WERROR_H__ */

/*
 * $Log: werror.h,v $
 * Revision 1.1  2003/11/18 01:50:09  radford
 * o On windows you can pass GetLastErorr() to werror() instead of error()
 *   to print out system messages.
 *
 */
