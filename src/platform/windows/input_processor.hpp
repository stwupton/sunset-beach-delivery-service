#pragma once

#include <Windows.h>

#include "common/input.hpp"

class InputProcessor {
protected:
	HWND windowHandle;
	HCURSOR cursors[(size_t)Cursor::_length];

public:
	void initialise(HWND windowHandle) {
		this->windowHandle = windowHandle;

		this->cursors[(size_t)Cursor::arrow] = LoadCursor(NULL, IDC_ARROW);
		this->cursors[(size_t)Cursor::pointer] = LoadCursor(NULL, IDC_HAND);
	}

	void process(Input *input) {
		POINT cursorPosition;
		GetCursorPos(&cursorPosition);
		ScreenToClient(this->windowHandle, &cursorPosition);
		input->previousMouse = input->mouse;
		input->mouse = Vec2<f32>(cursorPosition.x, cursorPosition.y);

		input->primaryButton.wasDown = input->primaryButton.down;
		input->primaryButton.down = GetKeyState(VK_LBUTTON) & 0x80;

		if (!input->primaryButton.wasDown && input->primaryButton.down) {
			input->primaryButton.start = input->mouse;
		}

		if (input->primaryButton.wasDown && !input->primaryButton.down) {
			input->primaryButton.end = input->mouse;
		}
	}

	void updateCursor(const Cursor &cursor) {
		SetCursor(this->cursors[(size_t)cursor]);
	}
};