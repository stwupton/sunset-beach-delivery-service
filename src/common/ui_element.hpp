#pragma once

#include "types/array.hpp"
#include "types/core.hpp"
#include "types/string.hpp"
#include "types/vector.hpp"
#include "utils/collision.hpp"

enum class UIType : u8 {
	text,
	line,
	circle,
	button
};

struct UICommonData {
	Rgba color;
};

enum class UITextAlignment {
	start,
	middle,
	end
};

struct UITextData : UICommonData {
	String16<100> text;
	String16<32> font;
	f32 fontSize; 
	Vec2<f32> position;
	f32 width, height;
	UITextAlignment horizontalAlignment = UITextAlignment::start;
	UITextAlignment verticalAlignment = UITextAlignment::start;
};

struct UILineData : UICommonData {
	f32 thickness;
	Vec2<f32> start;
	Vec2<f32> end;
};

enum class UICircleStrokeStyle : u8 {
	solid,
	dotted
};

struct UICircleData : UICommonData {
	f32 thickness;
	f32 radius;
	Vec2<f32> position;
	UICircleStrokeStyle strokeStyle = UICircleStrokeStyle::solid;
	Rgba strokeColor;
};

struct UIButtonLabelData : UICommonData {
	f32 fontSize;
	String16<32> text;
};

enum class UIButtonInputState : u8 {
	none = 0,
	over = 1 << 1,
	down = 1 << 2,
	clicked = 1 << 3
};

struct UIButtonData : UICommonData {
	UIButtonLabelData label;
	Vec2<f32> position;
	f32 width, height;
	u8 inputState = (u8)UIButtonInputState::none;

	void handleInput(const Input &input) {
		this->inputState = 0;

		if (boxCollision(input.mouse, this->position, this->width, this->height)) {
			this->inputState |= (u8)UIButtonInputState::over;

			const bool startedInButton = boxCollision(
				input.primaryButton.start, 
				this->position, 
				this->width, 
				this->height
			);

			if (input.primaryButton.down && startedInButton) {
				this->inputState |= (u8)UIButtonInputState::down;
			} else if (
				input.primaryButton.wasDown && 
				startedInButton &&
				boxCollision(input.primaryButton.end, this->position, this->width, this->height)
			) {
				this->inputState |= (u8)UIButtonInputState::clicked;
			}
		}
	}

	bool checkInput(UIButtonInputState state) const {
		return this->inputState & (u8)state;
	}
};

struct UIElement {
	UIType type;
	union {
		UICommonData common;
		UITextData text;
		UILineData line;
		UICircleData circle;
		UIButtonData button;
	};
};