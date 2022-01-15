#ifndef NDEBUG
	#define DEBUG 1
#endif

#if WIN32 
	#define UNICODE 1
	#define _USE_MATH_DEFINES 1
	#include "platform/windows/entry.hpp"
#else
	#error "No platform macro defined. Available platforms are: WIN32"
#endif