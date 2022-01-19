#pragma once

#include <cmath>

#include "common/game_state.hpp"
#include "game/utils.hpp"
#include "types/core.hpp"
#include "types/vector.hpp"

namespace SystemView {
	// Forward declerations
	void drawCentralStar(GameState *gameState);
	void drawLocations(GameState *gameState, f32 delta);

	const f32 starRadius = 50.0f;

	void setup(GameState *gameState) {
		gameState->textureLoadQueue.push(TextureAssetId::background);

		SystemLocation location = {};
		location.name = L"Arrakis";
		location.color = Rgba(0.8f, 0.64f, 0.3f, 1.0f);
		location.radius = 5.0f;
		location.orbitingDistance = 200.0f;
		location.orbitAngle = 2.5f;
		gameState->systemLocations.push(location);

		location.name = L"Caladan";
		location.color = Rgba(0.11f, 0.64f, 0.62f, 1.0f);
		location.radius = 7.0f;
		location.orbitingDistance = 500.0f;
		location.orbitAngle = 1.14f;
		const SystemLocation &caladan = gameState->systemLocations.push(location);

		location.name = L"Boobies";
		location.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
		location.radius = 2.0f;
		location.orbitingDistance = 10.0f;
		location.orbitAngle = 2.0f;
		location.orbiting = &caladan;
		gameState->systemLocations.push(location);

		location.name = L"Space Station";
		location.color = Rgba(0.6f, 0.6f, 0.6f, 1.0f);
		location.radius = 3.0f;
		location.orbitingDistance = 350.0f;
		location.orbitAngle = 0.1f;
		location.orbiting = nullptr;
		gameState->systemLocations.push(location);
	}

	void update(GameState *gameState, f32 delta) {
		Sprite background = {};
		background.assetId = TextureAssetId::background;
		background.position = Vec3<f32>(0.0f, 0.0f, 0.9f);
		background.scale = Vec2<f32>(1.3f, 1.3f);
		gameState->sprites.push(background);

		drawCentralStar(gameState);
		drawLocations(gameState, delta);

		if (gameState->input.primaryButton.down && gameState->pendingMode != GameModeId::combat) {
			gameState->pendingMode = GameModeId::combat;
			gameState->soundLoadQueue.push(SoundAssetId::stereo);
		}
	}

	void drawCentralStar(GameState *gameState) {
		UICircleData star = {};
		star.color = Rgba(0.99f, 1.0f, 0.0f, 1.0f);
		star.position = gameToScreen(Vec3<f32>());
		star.radius = starRadius;
		gameState->uiElements.push(star);
	}

	void drawLocations(GameState *gameState, f32 delta) {
		for (SystemLocation &location : gameState->systemLocations) {
			UICircleData circle = {};
			circle.color = location.color;
			circle.radius = location.radius;

			// Calculate position in orbit
			{
				location.orbitAngle += location.orbitSpeed * delta / location.orbitingDistance;
				circle.position = Vec2(1.0f, 0.0f);

				const f32 ca = cos(location.orbitAngle);
				const f32 sa = sin(location.orbitAngle);
				circle.position = Vec2(
					ca * circle.position.x - sa * circle.position.y, 
					sa * circle.position.x + ca * circle.position.y
				);

				f32 minRadius = starRadius;
				Vec2 offset = Vec2<f32>();
				if (location.orbiting != nullptr) {
					minRadius = location.orbiting->radius;
					offset = location.orbiting->position;
				}

				circle.position = circle.position * (minRadius + location.orbitingDistance) + offset;
				location.position = circle.position; // Position used for orbiting moons
				circle.position = gameToScreen(Vec3(circle.position.x, circle.position.y));
			}

			const f32 distance = gameState->input.mouse.distanceTo(circle.position);
			const bool mouseIsOver = distance <= location.radius + 10.0f;
			if (mouseIsOver) {
				circle.thickness = 1.0f;
				circle.strokeColor = Rgba(0.0f, 1.0f, 0.0f, 1.0f);
			}

			gameState->uiElements.push(circle);

			if (mouseIsOver) {
				UITextData infoText = {};
				infoText.text = location.name.data;
				infoText.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
				infoText.fontSize = 30.0f;
				infoText.position = circle.position;
				infoText.height = 30.0f;
				infoText.width = 200.0f;
				gameState->uiElements.push(infoText);
			}
		}
	}
};