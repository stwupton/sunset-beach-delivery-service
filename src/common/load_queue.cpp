#pragma once

#include <cmath>

#include "common/asset_definitions.hpp"
#include "types/array.hpp"
#include "types/core.hpp"

struct LoadQueue : Array<TextureAssetId, 8> {
	u8 toLoad = 0;

	void clear() {
		Array::clear();
		this->toLoad = 0;
	}

	f32 loadPercentage() const {
		return abs((f32)(this->length / this->toLoad) - 1);
	}

	void push(TextureAssetId id) {
		Array::push(id);
		toLoad++;
	}
};