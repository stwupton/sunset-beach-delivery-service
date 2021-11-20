#pragma once

#include <cassert>

template<typename T, size_t Size>
struct Array {
	size_t length = 0;
	T data[Size];

	T &operator [](size_t index) {
		assert(index >= 0 && index < Size);
		return this->data[index];
	}

	T pop() {
		return this->data[--this->length];
	}

	void push(T value) {
		assert(this->length != Size);
		this->data[this->length++] = value;
	}

	void clear() {
		this->length = 0;
	}
};