# $Header: /cvs/host/zzip/Makefile.am,v 1.11 2005/02/17 04:16:09 david Exp $
# -------=====================<<<< COPYRIGHT >>>>========================-------
#          Copyright (c) 2001 Indigita Corp,  All Rights Reserved.
#  See full text of copyright notice and limitations of use in file COPYRIGHT.h
# -------================================================================-------

AUTOMAKE_OPTIONS = foreign dist-zip 1.6
if NEED_ERROR_H
zzip_CFLAGS =  -Ierror
ERROR_C = error/error.c
endif
#SUBDIRS = zheader blowfish
bin_PROGRAMS = zzip zinfo zunzip
zunzip_SOURCES = $(zzip_SOURCES)
zzip_SOURCES = zzip.c zheader/zheader.c blowfish/blowfish.c $(ERROR_C)
EXTRA_zzip_SOURCES = error/error.c
zzip_LDFLAGS = -lz
#zzip_INCLUDES = @ERROR_INC@
zinfo_SOURCES = zinfo.c zheader/zheader.c
noinst_HEADERS = COPYRIGHT.h zheader/zheader.h blowfish/blowfish.h
EXTRA_DIST = makefile.rpm zzip.spec.in

CLEANFILES = -r RPM/ zzip.spec

CFLAGS = -Iblowfish -Izheader

zzip_CPPFLAGS = 
if HAVE_BZLIB2
zzip_CPPFLAGS += -DBZIP2
zzip_LDFLAGS += -lbz2
endif

# --- This is the RPM section:
version=$(VERSION)
rpmproj = zzip
copy_tarball = yes
#include makefile.rpm

zunzip$(EXEEXT): zzip
	ln -sf zzip zunzip

zzip-$(version).tar.gz: dist

zzip.spec : zzip.spec.in
	perl -pe 's/version .*/version $(version)/' $< > $@
# --- this concludes the RPM section.

bootstrap:
	aclocal
	autoconf
	automake --foreign --add-missing



# $Log: Makefile.am,v $
# Revision 1.11  2005/02/17 04:16:09  david
# - Added support for bzip2 compression library.
#
# Revision 1.10  2004/05/26 05:47:51  david
# - Apparently the min is 1.6 not 1.7. At least 1.6 works with Mac OS X.
#
# Revision 1.9  2004/05/12 20:21:04  radford
# o Specify the minimum automake version
#
# Revision 1.8  2003/12/24 21:49:20  david
# - Support for computers without error.h
# - Removed weird copying from subdir technique.
# - Fixed automake warning.
#
# Revision 1.7  2003/09/08 23:59:20  radford
# o Remove zsplit, zupload
#
# Revision 1.6  2003/09/05 00:59:40  radford
# o Add sources for zunzip
#
# Revision 1.5  2003/07/29 08:21:04  radford
# o Add zsplit and zupload
#
# Revision 1.4  2003/07/23 01:29:39  radford
# o force the link
#
# Revision 1.3  2003/07/19 02:15:40  david
# - Created zunzip link from zzip.
#
# Revision 1.2  2001/12/14 00:52:45  david
# - Now supports zheader in a separate directory.
# - Now supports blowfish.
# - Has neat header/footers.
#
