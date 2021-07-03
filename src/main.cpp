#if WIN32 
#include "platform/windows/entry.cpp"
#else
#error "No platform macro defined. Available platforms are: WIN32"
#endif