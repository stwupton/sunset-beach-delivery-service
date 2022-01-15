#pragma once

#include <wchar.h>

template<size_t Size>
struct String16 {
	wchar_t data[Size];

	String16() = default;

	String16(const wchar_t *str) {
		wcscpy_s(this->data, Size, str);
	}

	void operator =(const wchar_t *str) {
		wcscpy_s(this->data, Size, str);
	}
};