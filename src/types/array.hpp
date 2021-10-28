#pragma once

template<typename T, size_t Size>
struct Array {
	size_t length = 0;
	T data[Size];

	T &operator[](size_t index) const {
		return this->data[index];
	}

	void push(T value) {
		this->data[this->length++] = value;
	}

	void clear() {
		this->length = 0;
	}
};