#pragma once

#include <cstdio>

#include <Windows.h>

// NOTE(steven): Buffer size of 500 characters is completely arbitrary, 
// increase if needed
#define LOG(_x, ...)                    \
{                                       \
	WCHAR _buffer[500];                   \
	swprintf_s(_buffer, _x, __VA_ARGS__); \
	OutputDebugString(_buffer);            \
}

// TODO(steven): Keeping for now as it's useful for the render work that I'm 
// doing but it should be removed at some point
#define LOG_MATRIX(_m)                                 \
	LOG(L"%f, %f, %f, %f\n", _m.x0, _m.x1, _m.x2, _m.x3) \
	LOG(L"%f, %f, %f, %f\n", _m.y0, _m.y1, _m.y2, _m.y3) \
	LOG(L"%f, %f, %f, %f\n", _m.z0, _m.z1, _m.z2, _m.z3) \
	LOG(L"%f, %f, %f, %f\n", _m.w0, _m.w1, _m.w2, _m.w3)

#define RELEASE_COM_OBJ(_x) \
	if (_x != NULL) {         \
		_x->Release();          \
		_x = NULL;              \
	}