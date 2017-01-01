
#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

//#include "shiny\Shiny.h"
#include <assert.h>

#ifndef PROFILE_FUNC
#define PROFILE_FUNC()
#endif

#ifndef PROFILE_BLOCK
#define PROFILE_BLOCK(x)
#endif

#ifndef PROFILER_UPDATE
#define PROFILER_UPDATE()
#endif

#ifndef PROFILER_OUTPUT_TREE_STRING
#define PROFILER_OUTPUT_TREE_STRING()
#endif


#ifndef NULL
#define NULL	0
#endif
typedef unsigned int uint;
typedef unsigned int uint32;
typedef          int int32;
//typedef          char u8;

#ifdef _WIN32
typedef          __int64 i64;
typedef unsigned __int64 u64;
#else
typedef long long i64;
typedef unsigned long long u64;
#endif
#endif
