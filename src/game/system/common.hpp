#pragma once

#include "common/game_state.hpp"

namespace SystemCommon {
	void drawCentralStar(GameState *gameState, Vec2<f32> center, f32 radius) {
		UICircleData star = {};
		star.color = Rgba(0.99f, 1.0f, 0.0f, 1.0f);
		star.position = center;
		star.radius = radius;
		gameState->uiElements.push(star);
	}

	void drawStarField(GameState *gameState) {
		Sprite background = {};
		background.assetId = TextureAssetId::background;
		background.position = Vec3<f32>(0.0f, 0.0f, 0.9f);
		background.scale = Vec2<f32>(1.3f, 1.3f);
		gameState->sprites.push(background);
	}

	void updateOrbitLocation(SystemLocation *location, f32 delta) {
		location->orbit.angle += location->orbit.speed * delta / location->orbit.distance;
		location->orbit.angle = fmod(location->orbit.angle, M_PI * 2);
	}
};