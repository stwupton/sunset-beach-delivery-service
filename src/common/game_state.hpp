#pragma once

#include <cmath>

#include "common/asset_definitions.hpp"
#include "common/sprite.hpp"
#include "common/input.hpp"
#include "common/ui_element.hpp"
#include "types/array.hpp"

typedef Array<Sprite, 10> SpriteBuffer;

struct UIElementBuffer : Array<UIElement, 10> {
	void pushText(const UITextData &text) {
		UIElement element = {};
		element.type = UIType::text;
		memcpy(element.data, &text, sizeof(UITextData));
		this->push(element);
	}

	void pushLine(const UILineData &line) {
		UIElement element = {};
		element.type = UIType::line;
		memcpy(element.data, &line, sizeof(UILineData));
		this->push(element);
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