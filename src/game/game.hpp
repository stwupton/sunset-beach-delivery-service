#pragma once

#include <cmath>

#include "common/game_state.hpp"
#include "game/combat.hpp"
#include "game/system_view.hpp"

namespace Game {
	// Forward declerations
	void debugUI(GameState *gameState, f32 delta);

	void setup(GameState *gameState) {
		gameState->modes[(size_t)GameModeId::combat] = { &Combat::setup, &Combat::update };
		gameState->modes[(size_t)GameModeId::systemView] = { &SystemView::setup, &SystemView::update };

		gameState->modes[(size_t)gameState->mode].setup(gameState);
	}

	void update(GameState *gameState, f32 delta) {
#ifdef DEBUG
		delta *= gameState->gameSpeed;
#endif

		if (gameState->pendingMode != GameModeId::none) {
			gameState->modes[(size_t)gameState->pendingMode].setup(gameState);

			gameState->mode = gameState->pendingMode;
			gameState->pendingMode = GameModeId::none;

			return;
		}

		gameState->modes[(size_t)gameState->mode].update(gameState, delta);

#ifdef DEBUG
		debugUI(gameState, delta);
#endif
	}

	void debugUI(GameState *gameState, f32 delta) {
		UIElementBuffer &uiElements = gameState->uiElements;

		wchar_t textBuffer[100] = {};

		u8 fps = round(1 / delta);
		swprintf_s(textBuffer, L"FPS: %d", fps);
		UITextData text = {};
		text.text = textBuffer;
		text.fontSize = 30.0f;
		text.position = 0;
		text.width = 300.0f;
		text.height = 40.0f;
		text.color = Rgba(1.0f, 0.0f, 0.0f, 1.0f);
		uiElements.push(text);

		swprintf_s(textBuffer, L"Mouse Down: %d", gameState->input.primaryButton.down);
		text.text = textBuffer;
		text.position.y += 40.0f;
		uiElements.push(text);

		const Vec2<f32> mouse = gameState->input.mouse;
		swprintf_s(textBuffer, L"X: %d, Y: %d", (u32)mouse.x, (u32)mouse.y);
		text.text = textBuffer;
		text.position.y += 40.0f;
		uiElements.push(text);

		swprintf_s(textBuffer, L"Game Speed: x%u", gameState->gameSpeed);
		text.text = textBuffer;
		text.position.y += 40.0f;
		uiElements.push(text);
	}
};