/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* win32/version.h.  Generated by hand by Jeff Myers 9-15-03 */

/* This is the windows version stuff, it should pull from a resouce later */

#ifndef _GETBUILD_DATE_
#define _GETBUILD_DATE_
#include <stdio.h>
#include <string.h>

// to get the version in the right format YYYYMMDD
// yes this is horible but it needs to be done to get it right
inline int getBuildDate()
{
	int year = 1900,month = 0,day = 0;
	char monthStr[512];
	sscanf(__DATE__,"%s %d %d",monthStr,&day,&year);

	// we want it not as a name but a number
	if (strcmp(monthStr,"Jan")==0)
		month = 1;
	else if (strcmp(monthStr,"Feb")==0)
		month = 2;
	else if (strcmp(monthStr,"Mar")==0)
		month = 3;
	else if (strcmp(monthStr,"Apr")==0)
		month = 4;
	else if (strcmp(monthStr,"May")==0)
		month = 5;
	else if (strcmp(monthStr,"Jun")==0)
		month = 6;
	else if (strcmp(monthStr,"Jul")==0)
		month = 7;
	else if (strcmp(monthStr,"Aug")==0)
		month = 8;
	else if (strcmp(monthStr,"Sep")==0)
		month = 9;
	else if (strcmp(monthStr,"Oct")==0)
		month = 10;
	else if (strcmp(monthStr,"Nov")==0)
		month = 11;
	else if (strcmp(monthStr,"Dec")==0)
		month = 12;

	return (year*10000) + (month*100)+ day;
}
#endif //_GETBUILD_DATE_
// protocol version
#ifndef BZ_PROTO_VERSION
#define BZ_PROTO_VERSION "109a"
#endif //BZ_PROTO_VERSION

#ifndef BZ_MAJOR_VERSION
#define BZ_MAJOR_VERSION	1
#endif

#ifndef BZ_MINOR_VERSION
#define BZ_MINOR_VERSION	9
#endif

#ifndef BZ_REV
#define BZ_REV				0
#endif

#ifndef BZ_BUILD_OS
#ifdef _DEBUG
#define BZ_BUILD_OS			"W32VCD"
#else
#define BZ_BUILD_OS			"W32VC"
#endif
#endif

#ifndef BZ_BUILD_SOURCE
#define BZ_BUILD_SOURCE		 "CVS"
#endif

#ifndef BZ_BUILD_DATE
#define BZ_BUILD_DATE		getBuildDate()
#endif
