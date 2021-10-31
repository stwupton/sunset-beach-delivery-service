#pragma once

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

struct GameState {
	SpriteBuffer sprites;
	UIElementBuffer uiElements;
	Input input;
};