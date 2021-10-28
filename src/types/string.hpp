#pragma once

#include <wchar.h>

#include "types/core.hpp"

template<u32 Size>
struct string16 {
	wchar_t data[Size];

	void operator =(const wchar_t *str) {
		const u32 length = wcslen(str);
		memcpy(&this->data, str, (length + 1) * sizeof(wchar_t));
	}
};