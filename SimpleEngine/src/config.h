#ifndef _CONFIG_H_INCLUDED
#define _CONFIG_H_INCLUDED

// Windows
#ifdef _WIN32

#define SYSTEM_WIN32 1

#define USE_DX10_RENDERER 0
#define USE_GLES_RENDERER 1
#define USE_CPU_RENDERER 0

// Android
#elif defined( __ANDROID__ )

#define SYSTEM_ANDROID 1

#define USE_DX10_RENDERER 0
#define USE_GLES_RENDERER 1
#define USE_CPU_RENDERER 0

#endif

#endif


