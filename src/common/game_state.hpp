#pragma once

#include <cmath>

#include "common/asset_definitions.hpp"
#include "common/sprite.hpp"
#include "common/input.hpp"
#include "common/ui_element.hpp"
#include "types/array.hpp"

typedef Array<Sprite, 10> SpriteBuffer;

struct UIElementBuffer : Array<UIElement, 10> {
	void push(const UITextData &textData) {
		UIElement element = {};
		element.type = UIType::text;
		element.text = textData;
		Array::push(element);
	}

	void push(const UILineData &lineData) {
		UIElement element = {};
		element.type = UIType::line;
		element.line = lineData;
		Array::push(element);
	}
};

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

struct GameState {
	LoadQueue loadQueue;
	SpriteBuffer sprites;
	UIElementBuffer uiElements;
	Input input;
};