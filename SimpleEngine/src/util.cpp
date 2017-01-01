/*************************************************************************************
  Phyzaks
  Copyright (c) 2012 John Rohrssen
  Email: johnrohrssen@gmail.com
*************************************************************************************/

#include "config.h"
#include "util.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#if SYSTEM_WIN32 | SYSTEM_WIN32_DLL

#elif SYSTEM_ANDROID

#endif

int util_snprintf( char* dest, size_t size, const char* format, ... )
{
	va_list args;
	va_start (args, format);
	int ret = vsnprintf( dest, size, format, args );
	va_end(args);;
	return ret;
}

char* util_strtok( char* str, const char* format )
{
#if SYSTEM_WIN32 | SYSTEM_WIN32_DLL | SYSTEM_WIN32_QT_TOOL
	return strtok( str, format );
#elif SYSTEM_ANDROID
	return strtok( str, format );
#else
#error Unsupported platform
#endif
}
