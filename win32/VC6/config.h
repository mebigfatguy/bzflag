/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* win32/config.h.  Generated by hand by Jeff Myers 6-12-03 */
/* this config is just for visual C++ since it donsn't use automake*/

#ifndef _VC5_6_CONFIG
#define _VC5_6_CONFIG

/* Time Bomb expiration */
/* #undef TIME_BOMB */

/* Debug Rendering */
/* #undef DEBUG_RENDERING */

/* Enabling Robots */
#define ROBOT 1

/* Enabling Snapping */
#define SNAPPING 1

/* On windows, strcasecmp is really stricmp */
#define HAVE_STRICMP 1

// define our OS
#ifndef BZ_BUILD_OS
 	#if defined(_MSC_VER) && (_MSC_VER == 1100)
 		#ifdef _DEBUG
 			#define BZ_BUILD_OS			"W32VC5D"
 		#else
 			#define BZ_BUILD_OS			"W32VC5"
 		#endif //_DEBUG
 	#endif //_MSC_VER == 1100
 	#if defined(_MSC_VER) && (_MSC_VER == 1200)
 		#ifdef _DEBUG
 			#define BZ_BUILD_OS			"W32VC6D"
 		#else
 			#define BZ_BUILD_OS			"W32VC6"
 		#endif //_DEBUG
 	#endif //_MSC_VER == 1200
#endif

#ifndef DEBUG
  #ifdef _DEBUG
	  #define DEBUG 1
  #else
	  #define DEBUG 0
  #endif
#endif

#define WINVER 0x0400
#define _WIN32_WINNT 0x0400

#if(1)
  #include "ATLbase.h"
  #if defined(_MSC_VER) && (_MSC_VER == 1200)
    inline void W32_DEBUG_TRACE (const char* buffer ) {ATLTRACE(buffer);}
  #else ifdef(_MSC_VER) && (_MSC_VER == 1100)
    inline void W32_DEBUG_TRACE (const char* buffer ) {ATLTRACE(buffer);}
  #endif
#else
  inline void W32_DEBUG_TRACE (const char* buffer){return;}
#endif

#endif //_VC5_6_CONFIG=======
