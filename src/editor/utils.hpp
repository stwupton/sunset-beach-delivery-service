#pragma once

UIButtonData uiButton(const wchar_t *label, Vec2<f32> position, const Input &input) {
	UIButtonData button = {};
	button.label.text = label;
	button.label.fontSize = 30.0f;
	button.label.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
	button.color = Rgba(0.3f, 0.3f, 0.3f, 1.0f);
	button.height = 100.0f;
	button.width = 200.0f;
	button.position = position;

	button.handleInput(input);
	if (button.checkInput(UIButtonInputState::over)) {
		button.color += Rgba(0.1f, 0.1f, 0.1f);

		if (button.checkInput(UIButtonInputState::down)) {
			const f32 scale = 0.9f;
			button.label.fontSize *= scale;
			button.position += Vec2(
				button.width - button.width * scale, 
				button.height - button.height * scale
			) * 0.5f;
			button.width *= scale;
			button.height *= scale;
		}
	}

	return button;
}