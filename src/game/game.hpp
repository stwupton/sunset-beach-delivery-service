#pragma once

#include <cmath>

#include "common/game_state.hpp"
#include "game/combat.hpp"
#include "game/system/system_select.hpp"
#include "game/system/system_view.hpp"
#include "game/update_tweens.hpp"

namespace Game {
	// Forward declerations
	void debugUI(GameState *gameState, f32 delta);
	void populateSystemLocations(GameState *gameState);

	void setup(GameState *gameState) {
		// TODO(steven): Get from load data instead
		gameState->playerShip.fuelTankCapacity = 30.0f;
		gameState->playerShip.fuel = 30.0f;
		gameState->dockedLocation = &gameState->systemLocations[0];

		Shipment package1 = Shipment{};

		gameState->pendingMusicItem = MusicAssetId::mars;

		populateSystemLocations(gameState);
		SystemSelect::populateAvailablePackages(gameState);
		SystemSelect::setup(gameState);
	}

	void update(GameState *gameState, f32 delta) {
#ifdef DEBUG
		const f32 realDelta = delta;
		delta *= gameState->gameSpeed;
#endif
		for (UpdateSystem system : gameState->updateSystems) {
			system(gameState, delta);
		}

		updateTweens(gameState, delta);
#ifdef DEBUG
		debugUI(gameState, realDelta);
#endif
	}

	void debugUI(GameState *gameState, f32 delta) {
		UIElementBuffer &uiElements = gameState->uiElements;

		wchar_t textBuffer[100] = {};

		u8 fps = round(1 / delta);
		swprintf_s(textBuffer, L"FPS: %d", fps);
		UITextData text = {};
		text.text = textBuffer;
		text.font = L"consolas";
		text.fontSize = 16.0f;
		text.position = 0;
		text.width = 300.0f;
		text.height = 20.0f;
		text.color = Rgba(0.0f, 1.0f, 0.0f, 1.0f);
		uiElements.push(text);

		swprintf_s(textBuffer, L"Mouse Down: %d", gameState->input.primaryButton.down);
		text.text = textBuffer;
		text.position.y += text.height;
		uiElements.push(text);

		const Vec2<f32> mouse = gameState->input.mouse;
		swprintf_s(textBuffer, L"X: %d, Y: %d", (u32)mouse.x, (u32)mouse.y);
		text.text = textBuffer;
		text.position.y += text.height;
		uiElements.push(text);

		swprintf_s(textBuffer, L"Game Speed: x%u", gameState->gameSpeed);
		text.text = textBuffer;
		text.position.y += text.height;
		uiElements.push(text);
	}

	void populateSystemLocations(GameState *gameState) {
		SystemLocation location = {};
		location.name = L"Arrakis";
		location.color = Rgba(0.8f, 0.64f, 0.3f, 1.0f);
		location.radius = 20.0f;
		location.orbit.angle = 2.5f;
		location.orbit.distance = 200.0f;
		gameState->systemLocations.push(location);

		location.name = L"Caladan";
		location.color = Rgba(0.11f, 0.64f, 0.62f, 1.0f);
		location.radius = 30.0f;
		location.orbit.angle = M_PI + M_PI / 4;
		location.orbit.distance = 1000.0f;
		gameState->systemLocations.push(location);

		location.name = L"Boobies";
		location.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
		location.radius = 5.0f;
		location.orbit.angle = 2.0f;
		location.orbit.distance = 10.0f;
		location.isMoon = true;
		gameState->systemLocations.push(location);

		location.name = L"Space Station";
		location.color = Rgba(0.6f, 0.6f, 0.6f, 1.0f);
		location.radius = 5.0f;
		location.orbit.angle = 0.1f;
		location.orbit.distance = 2000.0f;
		location.isMoon = false;
		gameState->systemLocations.push(location);
	}
};