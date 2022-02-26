#pragma once

#include <cmath>

#include "common/game_state.hpp"
#include "types/vector.hpp"
#include "game/system/common.hpp"
#include "game/system/system_select.hpp"
#include "game/utils.hpp"
#include "types/core.hpp"

namespace SystemSelect { void setup(GameState *gameState); };

namespace SystemView {
	// Forward declerations
	void drawLocations(GameState *gameState, f32 delta);
	void update(GameState *gameState, f32 delta);

	const f32 scale = 0.2f;
	const f32 starRadius = 400.0f;
	const Vec2<f32> starCenter = gameToScreen(Vec3<f32>());

	void setup(GameState *gameState) {
		gameState->textureLoadQueue.push(TextureAssetId::background);

		gameState->updateSystems.clear();
		gameState->updateSystems.push(&update);
	}

	void update(GameState *gameState, f32 delta) {
		SystemCommon::drawStarField(gameState);

		if (gameState->input.keyDown == '\t') {
			SystemSelect::setup(gameState);
		}

		SystemCommon::drawCentralStar(gameState, starCenter, starRadius * scale);
		drawLocations(gameState, delta);
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

			SystemCommon::updateOrbitLocation(&location, delta);
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