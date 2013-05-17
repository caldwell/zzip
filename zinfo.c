/* $Header: /cvs/host/zzip/zinfo.c,v 1.4 2003/07/24 04:24:53 david Exp $*/
#undef LZOLibNotZlib
#ifdef LZOLibNotZlib
#define Extension ".lzo"
#else
#define Extension ".z"
#endif
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <time.h>

typedef unsigned long dword;

#include "zheader.h"

void Usage(char *me)
{
	fprintf(stderr,"usage: %s <ram-compressed-image> [-h<header>]\n",me);
	exit(1);
}

int main(int c,char **v)
{
	int r,e;
	char *extraHeader[c],*filename=0;
	long extraHeaders=0;
	FILE *f;

#ifdef LZOLibNotZlib
	lzo_uint zlen;
	void *workmem;
#else
	unsigned long zlen;
#endif
	unsigned char *rom,*zrom,*zfile;
	struct stat s;

	/* parse command line */
	while (--c)	/* we must skip first argument (0) as that is OUR program name, so we pre-decrement c */
		if (v[c][0] == '-')
			if (v[c][1] == 'h') /* currently we only handle -h */
				extraHeader[extraHeaders++]=&v[c][2];
			else {
				fprintf(stderr,"'%c' is an invalid option\n",v[c][1]);
				Usage(v[0]);
			}
		else /* no dash must mean its a filename */
			if (!filename)
				filename = v[c];
			else {
				fprintf(stderr,"Only one file may be specified; can't compress \"%s\" AND \"%s\".\n",filename,v[c]);
				Usage(v[0]);
			}

	if (!filename) {
		fprintf(stderr,"You must specify a file.\n");
		Usage(v[0]);
	}

	r = open(filename, O_RDONLY
#ifdef MSDOS
		   |O_BINARY
#endif
			 );
	if(r==-1){
		fprintf(stderr,"%s: can't open %s\n",v[0],filename);
		exit(1);
	}
	if(fstat(r,&s) == -1) {
		fprintf(stderr,"%s: why did stat fail?\n",v[0]);
		exit(1);
	}
	rom   = malloc(s.st_size);

	if(rom == 0) {
		fprintf(stderr,"%s: No memory\n",v[0]);
		exit(1);
	}
	if(read(r,rom,s.st_size) != s.st_size) {
		fprintf(stderr,"%s: Can't read\n",v[0]);
		exit(1);
	}
	close(r);

	if (!IsValidHeader(rom)) {
		printf("Invalid zzip header\n");
		exit(1);
	}

	if (!extraHeaders)	
		printf("%s",rom);
	else {
		while (extraHeaders--) {
			char s[256];
			int found;
			dword x=0;
			found = GetHeaderString(rom,extraHeader[extraHeaders],s, sizeof(s));
			found = GetHeaderNumber(rom,extraHeader[extraHeaders],&x);
			printf("%s: %s (%d)\n",extraHeader[extraHeaders],found?s:"Not found.",x);
		}
	}

	exit (0);
}

/* $History: zinfo.c $
 * 
 * *****************  Version 3  *****************
 * User: Radford      Date: 7/27/98    Time: 5:06p
 * Updated in $/Host Applications/zzip
 * david:
 *   removed checksum(), put it into zheader.c
 * 
 * *****************  Version 2  *****************
 * User: David        Date: 1/29/97    Time: 3:11p
 * Updated in $/Sunflower/Loader/zzip
 * Removed header stuff into zheader.c file.
 */ 
