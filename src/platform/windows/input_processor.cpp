#pragma once

#include <Windows.h>

#include "input.hpp"

class InputProcessor {
public:
	void process(Input *input) {
		POINT cursorPosition;
		GetCursorPos(&cursorPosition);
		input->previousMouse.x = input->mouse.x;
		input->previousMouse.y = input->mouse.y;
		input->mouse.x = cursorPosition.x;
		input->mouse.y = cursorPosition.y;
	}
};