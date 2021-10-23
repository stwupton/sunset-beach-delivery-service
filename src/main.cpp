#ifndef NDEBUG
	#define DEBUG
#endif

#if WIN32 
	#define UNICODE
	#define _USE_MATH_DEFINES
	#include "platform/windows/entry.cpp"
#else
	#error "No platform macro defined. Available platforms are: WIN32"
#endif