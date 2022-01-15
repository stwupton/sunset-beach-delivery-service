#pragma once

#include <cmath>

#include "common/asset_definitions.hpp"
#include "types/array.hpp"
#include "types/core.hpp"

template<typename T, size_t Size>
struct LoadQueue : Array<T, Size> {
	u8 toLoad = 0;

	void clear() {
		Array<T, Size>::clear();
		this->toLoad = 0;
	}

	f32 loadPercentage() const {
		return abs((f32)(this->length / this->toLoad) - 1);
	}

	void push(T id) {
		Array<T, Size>::push(id);
		toLoad++;
	}
};