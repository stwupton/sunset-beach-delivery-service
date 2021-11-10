#pragma once

#include <wchar.h>

template<size_t Size>
struct string16 {
	wchar_t data[Size];

	void operator =(const wchar_t *str) {
		const size_t length = wcslen(str);
		memcpy(&this->data, str, (length + 1) * sizeof(wchar_t));
	}
};