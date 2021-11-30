#pragma once

#include <cstdio>

template<typename T>
struct Nullable : T {
	bool isNull = true;

	Nullable<T> &operator =(const T &x) {
		memcpy(this, &x, sizeof(T));
		this->isNull = false;
		return *this;
	}
};