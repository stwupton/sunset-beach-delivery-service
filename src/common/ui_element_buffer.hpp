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

	void push(const UIButtonData &buttonData) {
		UIElement element = {};
		element.type = UIType::button;
		element.button = buttonData;
		Array::push(element);

		// Create a text element out of the label data and add it after the button
		UIElement textElement = {};
		textElement.type = UIType::text;
		textElement.text.text = buttonData.label.text.data;
		textElement.text.color = buttonData.label.color;
		textElement.text.fontSize = buttonData.label.fontSize;
		textElement.text.width = buttonData.width;
		textElement.text.height = buttonData.height;
		textElement.text.position = buttonData.position;
		textElement.text.horizontalAlignment = UITextAlignment::middle;
		textElement.text.verticalAlignment = UITextAlignment::middle;
		Array::push(textElement);
	}
};