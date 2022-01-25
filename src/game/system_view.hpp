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

	const f32 scale = 0.2f;
	const f32 starRadius = 400.0f;
	const Vec2<f32> starCenter = gameToScreen(Vec3<f32>());

	void setup(GameState *gameState) {
		gameState->textureLoadQueue.push(TextureAssetId::background);
		gameState->pendingMusicItem = MusicAssetId::mars;

	}

	void update(GameState *gameState, f32 delta) {
		Sprite background = {};
		background.assetId = TextureAssetId::background;
		background.position = Vec3<f32>(0.0f, 0.0f, 0.9f);
		background.scale = Vec2<f32>(1.3f, 1.3f);
		gameState->sprites.push(background);

		if (gameState->input.keyDown == '\t') {
			gameState->nextMode = GameModeId::systemSelect;
		}

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
		star.position = starCenter;
		star.radius = starRadius * scale;
		gameState->uiElements.push(star);
	}

	void drawLocations(GameState *gameState, f32 delta) {
		u8 moonOffset = 0;

		for (size_t i = 0; i < gameState->systemLocations.length; i++) {
			SystemLocation &location = gameState->systemLocations[i];

			const f32 locationRadius = location.radius * scale;
			const f32 locationDistance = location.orbit.distance * scale;

			UICircleData circle = {};
			circle.color = location.color;
			circle.radius = locationRadius;

			location.orbit.angle += location.orbit.speed * delta / location.orbit.distance;
			circle.position = Vec2(1.0f, 0.0f);

			const f32 ca = cos(location.orbit.angle);
			const f32 sa = sin(location.orbit.angle);
			circle.position = Vec2(
				ca * circle.position.x - sa * circle.position.y, 
				sa * circle.position.x + ca * circle.position.y
			);

			f32 minRadius = starRadius * scale;
			Vec2 orbitCenter = starCenter;
			if (i != 0 && location.isMoon) {
				const SystemLocation &toOrbit = gameState->systemLocations[i - moonOffset - 1];
				minRadius = toOrbit.radius * scale;
				orbitCenter = toOrbit.position;

				moonOffset++;
			} else {
				moonOffset = 0;
			}

			// Orbit path
			{
				UICircleData orbitPath = {};
				orbitPath.strokeColor = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
				orbitPath.strokeStyle = UICircleStrokeStyle::solid;
				orbitPath.radius = minRadius + locationDistance;
				orbitPath.thickness = 1.0f;
				orbitPath.position = orbitCenter;
				gameState->uiElements.push(orbitPath);
			}

			circle.position = circle.position * (minRadius + locationDistance) + orbitCenter;
			location.position = circle.position; // Position used for orbiting moons
			circle.position = Vec3(circle.position.x, circle.position.y);

			const f32 distance = gameState->input.mouse.distanceTo(circle.position);
			const bool mouseIsOver = distance <= locationRadius + 10.0f;
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