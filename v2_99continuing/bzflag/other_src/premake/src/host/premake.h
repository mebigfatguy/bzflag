/**
 * \file   premake.h
 * \brief  Program-wide constants and definitions.
 * \author Copyright (c) 2002-2008 Jason Perkins and the Premake project
 */

#define lua_c
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"


/* Identify the current platform I'm not sure how to reliably detect
 * Windows but since it is the most common I use it as the default */
#if defined(__linux__)
#define PLATFORM_LINUX    (1)
#define PLATFORM_STRING   "linux"
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#define PLATFORM_BSD      (1)
#define PLATFORM_STRING   "bsd"
#elif defined(__APPLE__) && defined(__MACH__)
#define PLATFORM_MACOSX   (1)
#define PLATFORM_STRING   "macosx"
#elif defined(__sun__) && defined(__svr4__)
#define PLATFORM_SOLARIS  (1)
#define PLATFORM_STRING   "solaris"
#else
#define PLATFORM_WINDOWS  (1)
#define PLATFORM_STRING   "windows"
#endif


/* Pull in platform-specific headers required by built-in functions */
#if PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif


/* A success return code */
#define OKAY   (0)


/* Bootstrapping helper functions */
int do_isfile(const char* filename);


/* Built-in functions */
int path_isabsolute(lua_State* L);
int os_chdir(lua_State* L);
int os_copyfile(lua_State* L);
int os_getcwd(lua_State* L);
int os_isdir(lua_State* L);
int os_isfile(lua_State* L);
int os_matchdone(lua_State* L);
int os_matchisfile(lua_State* L);
int os_matchname(lua_State* L);
int os_matchnext(lua_State* L);
int os_matchstart(lua_State* L);
int os_mkdir(lua_State* L);
int os_pathsearch(lua_State* L);
int os_rmdir(lua_State* L);
int os_uuid(lua_State* L);
int string_endswith(lua_State* L);

