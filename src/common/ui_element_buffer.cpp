#pragma once

#include "common/ui_element.hpp"
#include "types/array.hpp"

struct UIElementBuffer : Array<UIElement, 100> {
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

	void push(const UICircleData &circleData) {
		UIElement element = {};
		element.type = UIType::circle;
		element.circle = circleData;
		Array::push(element);
	}
};