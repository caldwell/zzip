/* $Header: /cvs/host/zzip/zheader/zheader.c,v 1.18 2004/08/27 02:42:06 david Exp $
-------=====================<<<< COPYRIGHT >>>>========================-------
         Copyright (c) 1995-1998 Indigita Corp,  All Rights Reserved.
 See full text of copyright notice and limitations of use in file COPYRIGHT.h
-------================================================================-------
*/
#include "COPYRIGHT.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "zheader.h"

#define CR 13
#define LF 10
#define iseol(x) ((x)==CR || (x)==LF)

bool IsValidHeader(char *header)
{
	return (strncmp(header,"zzip",4) == 0);
}

bool GetHeaderNumber(char *header,char *name,unsigned long *value)
{
	if (!(header = LocateName(header,name)))
		return false;

	if (value)
        *value = strtoul(header,NULL,0);
	return true;
}

bool GetHeaderString(char *header,char *name, char *value, int length)
{
	if (!(header = LocateName(header,name)))
		return false;

	if (value && length) {
        while (*header && !iseol(*header) && --length)
            *value++ = *header++;
        *value = 0;
    }
	return true;
}

bool GetHeaderBoolean(char *header, char *name, bool *value)
{
    char b[24];
    if(!GetHeaderString(header,name,b,sizeof(b)))
        return false;

    if (strchr("Tt1Yy",b[0])) {
        if (value) *value = true;
    } else if (strchr("Ff0Nn",b[0])) {
        if (value) *value = false;
    } else
        return false;
    
    return true;
}

char *EatSpace(char *in)
{
	while (*in && strchr(" \t\n\r",*in)) /* End of lines are now considered space */
		in++;
	return in;
}

char *EatSpaceTillEOL(char *in)
{
	while (*in && strchr(" \t",*in)) /* End of lines are now considered space */
		in++;
	return in;
}

char *LocateName(char *header,char *name)
{
	while (1)
	{
		while (*header && !iseol(*header))	/* this skips the first line, on the first time through,
										       then skips the rest of each line that didn't match on subsequent
											   passes */
			header++;
		if (!*header) return NULL;
		header = EatSpace(header+1); /* space can be anywhere */
		if (!*header) return NULL;

		if (strncmp(header,name,strlen(name))==0) {
			header = EatSpace(header+strlen(name));
			if (*header != '=' &&
                *header != ':') continue; /* it wasn't really a match */
            header = EatSpaceTillEOL(header + 1);
			if (!*header) return NULL;
			return header;
		}
	}
	/* cant make it here, but to console the compiler:*/
	return NULL;
}

unsigned long HeaderLength(char *header)
{
	return strlen(header) + 1; /* Null terminated */
}

char *NextHeaderName(char *name, int *length)
{
    char *new;
  again:
    while (*name && !iseol(*name))
        name++;
    if (!*name) return NULL;
    
    new = name = EatSpace(name+1);
    if (!*name) return NULL;

    while (*name && !strchr("\n\r=:",*name))
        name++;
    if (!*name) return NULL;
    
    if (*name == '\r' || *name == '\n') {
        name++;
        goto again; /* crap! */
    }
    
    while (name > new && strchr(" \t",*--name))
        ;

    *length = name - new + 1;
    return new;
}

unsigned long Checksum(unsigned char *c,unsigned long l)
{
	unsigned long sum=0;
	while (l--)
		sum += *c++;
	return sum;
}

#if 0
/* I wrote these great functions, then realized I couldn't use them with our loaders stupid scatter-gather implementation. */
int NumberOfImagesInBundle(char *firstHeader, int maxLength)
{
    int count;

    for (count = 0;maxLength > 0;count++)
    {
        unsigned long length;
        if (!GetHeaderNumber(firstHeader, "Compressed Length", &length))
            break;
        length += HeaderLength(firstHeader);
        maxLength -= length;
        firstHeader += length;
    }
    return count;
}

char *GetImageFromBundle(char *firstHeader, int number)
{
    for (number;number != 0;number--)
    {
        unsigned long length;
        if (!GetHeaderNumber(firstHeader, "Compressed Length", &length))
            return NULL;
        length += HeaderLength(firstHeader);
        firstHeader += length;
    }
    return firstHeader;
}
#endif

/*
 * $Log: zheader.c,v $
 * Revision 1.18  2004/08/27 02:42:06  david
 * - Printing the header is easy enough... No need for a function.
 *
 * Revision 1.17  2004/04/30 23:54:25  radford
 * o Added a "Run Before Status" header to tell the rom-loader that it
 *   should wait for the uploaded program to run before returning status
 *   to the host allowing the return value of the program itself to
 *   be returned instead of only the status of the load.
 *
 * Revision 1.16  2003/10/20 22:58:55  radford
 * o Shrink GetHeaderBoolean by only checking the first character of the value.
 *
 * Revision 1.15  2003/10/13 23:28:02  radford
 * o Make the zheader and database interfaces clearer by using bools.
 *
 * Revision 1.14  2003/07/29 05:31:35  david
 * - Fixed rom-loader being too big.
 *
 * Revision 1.13  2003/07/29 01:04:57  radford
 * o Added GetHeaderBoolean with lots of different bool representations
 *   at David's insistence.
 *
 * Revision 1.12  2003/07/03 19:02:38  radford
 * o c99 is for people who don't run debian
 *
 * Revision 1.11  2003/06/27 23:09:50  radford
 * o Added command line completion for files, preferences and commands.
 *
 * Revision 1.10  2003/04/03 02:19:57  roy
 * o  Length being checked incorrectly.
 *
 * Revision 1.9  2003/03/24 18:56:17  david
 * - Fixed amazingly long-standing zheader bug that Dzanh caught.
 *   LocateName() could escape the string that it was searching in.
 *
 * Revision 1.8  2003/03/24 13:45:29  radford
 * o GetHeaderString now takes the length its buffer. This squashes a
 *   few buffer overflows in the rom-loader/loader/firmware.
 *
 * Revision 1.7  2002/08/23 04:22:51  david
 * - Fixed problem where zheader wouldn't handle "key =" with no value.
 *
 * Revision 1.6  2002/04/20 00:41:14  david
 * - Made EatSpace() and LocateName() non-static so the preferences code
 *   can use them.
 *
 * Revision 1.5  2002/03/30 06:52:04  david
 * - Allow passing the passing in of NULL to GetHeaderNumber() and
 *   GetHeaderString(). This can be used to check for the existance
 *   of a particular key in the header without getting it.
 *
 * Revision 1.4  2002/03/26 22:19:02  david
 * - Changed to CVS log instead of sourcesafe 'history'.
 *
 * 
 * *****************  Version 10  *****************
 * User: Roy          Date: 2/16/99    Time: 1:48p
 * Updated in $/Firmware/Forehead/Loader
 * o  Killed old style header.
 * 
 * *****************  Version 9  *****************
 * User: David        Date: 11/4/98    Time: 2:45 PM
 * Updated in $/Host Applications/scsi
 * dont want to print on macintosh. Or maybe we wanted to. Go check the
 * diff, I cant right now.
 * 
 * *****************  Version 8  *****************
 * User: Radford      Date: 7/27/98    Time: 7:10p
 * Updated in $/Firmware/Loader
 * David: Added checksum routine (was moved from zzip.c).
 * 
 * *****************  Version 7  *****************
 * User: Radford      Date: 4/03/98    Time: 5:40p
 * Updated in $/Sunflower/Host Applications/zzip
 * use printf nore k_printf on unix
 * 
 * *****************  Version 6  *****************
 * User: David        Date: 10/22/97   Time: 1:20a
 * Updated in $/Sunflower/Loader/SFF
 * I wrote these great functions, then realized I couldn't use them with
 * our loaders stupid scatter-gather implementation. Maybe someone else
 * can use them.
 * 
 * *****************  Version 5  *****************
 * User: David        Date: 2/11/97    Time: 3:38p
 * Updated in $/Sunflower/Loader/SFF
 * Added in code that doesn't care about EOL character.
 * Eat space after the '=' so that strings don't have spaces or tabs in
 * front of them.
 * 
 * *****************  Version 4  *****************
 * User: David        Date: 1/29/97    Time: 3:19p
 * Updated in $/Sunflower/Loader/SFF
 * strtoul -> strtol because hitachi library doesn't have strtoul.
 * ifdef for printfing out header inloader.
 * 
 * *****************  Version 3  *****************
 * User: David        Date: 1/21/97    Time: 5:42p
 * Updated in $/Sunflower/Loader/zzip
 * Header Length now includes null byte.
 * 
 * *****************  Version 2  *****************
 * User: David        Date: 1/21/97    Time: 5:40p
 * Updated in $/Sunflower/Loader/SFF
 * Fixed compile errors.
 * 
 * *****************  Version 1  *****************
 * User: David        Date: 1/21/97    Time: 3:35p
 * Created in $/Sunflower/Loader/zzip
 * This is a compresson utility that works with the loader so our firmware
 * fits in the flash ROM. yea!
 */ 
