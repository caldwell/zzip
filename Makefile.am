# $Header: /cvs/host/zzip/Makefile.am,v 1.7 2003/09/08 23:59:20 radford Exp $
# -------=====================<<<< COPYRIGHT >>>>========================-------
#          Copyright (c) 2001 Indigita Corp,  All Rights Reserved.
#  See full text of copyright notice and limitations of use in file COPYRIGHT.h
# -------================================================================-------

AUTOMAKE_OPTIONS = foreign dist-zip
#SUBDIRS = zheader blowfish
bin_PROGRAMS = zzip zinfo zunzip
zunzip_SOURCES = $(zzip_SOURCES)
zzip_SOURCES = zzip.c zheader.c blowfish.c
zinfo_SOURCES = zinfo.c zheader.c
zzip_LDFLAGS = -lz
noinst_HEADERS = COPYRIGHT.h zheader.h blowfish.h
EXTRA_DIST = makefile.rpm zzip.spec.in

CLEANFILES = -r RPM/ zzip.spec

# --- This is the RPM section:
version=$(VERSION)
rpmproj = zzip
copy_tarball = yes
#include makefile.rpm

zheader.c: zheader/zheader.c zheader.h
	cp $< $@

zheader.h: zheader/zheader.h
	cp $< $@

blowfish.c: blowfish/blowfish.c blowfish.h
	cp $< $@

blowfish.h: blowfish/blowfish.h
	cp $< $@

zunzip: zzip
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
