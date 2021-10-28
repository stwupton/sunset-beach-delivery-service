#pragma once

#include <Windows.h>

#include "common/input.hpp"

class InputProcessor {
public:
	void process(Input *input) {
		POINT cursorPosition;
		GetCursorPos(&cursorPosition);
		input->previousMouse = input->mouse;
		input->mouse = Vec2<f32>(cursorPosition.x, cursorPosition.y);

		const bool wasDown = input->primaryButton.down;
		input->primaryButton.down = GetKeyState(VK_LBUTTON) & 0x80;
		if (!wasDown && input->primaryButton.down) {
			input->primaryButton.start = input->mouse;
		}
	}
};