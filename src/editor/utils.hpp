#pragma once

#define UI_BUTTON(_varName, _text, _position)          \
	UIButtonData _varName = {};                          \
	_varName.label.text = _text;                         \
	_varName.label.fontSize = 30.0f;                     \
	_varName.label.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f); \
	_varName.color = Rgba(0.4f, 0.4f, 0.4f, 1.0f);       \
	_varName.height = 100.0f;                            \
	_varName.width = 200.0f;                             \
	_varName.position = _position;