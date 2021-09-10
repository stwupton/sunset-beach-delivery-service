#pragma once

#include <cstdio>

#include <Windows.h>

// NOTE(steven): Buffer size of 500 characters is completely arbitrary, 
// increase if needed
#define LOG(x, ...)                   \
{                                     \
	WCHAR buffer[500];                  \
	swprintf_s(buffer, x, __VA_ARGS__); \
	OutputDebugString(buffer);          \
}

#define RELEASE_COM_OBJ(x)      \
	if (x != NULL) {              \
		x->Release();               \
		x = NULL;                   \
	}