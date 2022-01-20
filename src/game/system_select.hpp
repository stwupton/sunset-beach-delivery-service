#pragma once

#include <cmath>

#include "common/game_state.hpp"
#include "game/utils.hpp"
#include "types/core.hpp"
#include "types/vector.hpp"

namespace SystemSelect {
	// Forward declerations
	void drawCentralStar(GameState *gameState);
	void drawLocations(GameState *gameState, f32 delta);

	const f32 starRadius = 400.0f;
	const Vec2<f32> starCenter = Vec3(-250.0f, 1080.0f * 0.5f);
	const f32 minPlanetSpacing = 100.0f;
	const f32 minMoonSpacing = minPlanetSpacing * 0.2f;

	void setup(GameState *gameState) {
		gameState->textureLoadQueue.push(TextureAssetId::background);
	}

	void update(GameState *gameState, f32 delta) {
		Sprite background = {};
		background.assetId = TextureAssetId::background;
		background.position = Vec3<f32>(0.0f, 0.0f, 0.9f);
		background.scale = Vec2<f32>(1.3f, 1.3f);
		gameState->sprites.push(background);

		if (gameState->input.keyDown == '\t') {
			gameState->nextMode = GameModeId::systemView;
		}

		drawCentralStar(gameState);
		drawLocations(gameState, delta);
	}

	void drawCentralStar(GameState *gameState) {
		UICircleData star = {};
		star.color = Rgba(0.99f, 1.0f, 0.0f, 1.0f);
		star.position = starCenter;
		star.radius = starRadius;
		gameState->uiElements.push(star);
	}

	void drawLocations(GameState *gameState, f32 delta) {
		u8 totalMoonOffset = 0;
		u8 moonOffset = 0;

		f32 previousPlanetRadius = starRadius;
		f32 previousMoonRadius = 0.0f;

		bool locationHighlighted = false;

		for (size_t i = 0; i < gameState->systemLocations.length; i++) {
			Vec2<f32> orbitCenter = Vec2<f32>();
			f32 distance = 0.0f;

			SystemLocation &location = gameState->systemLocations[i];

			if (i != 0 && location.isMoon) {
				const SystemLocation &toOrbit = gameState->systemLocations[i - moonOffset - 1];
				orbitCenter = toOrbit.position;
				distance = minMoonSpacing + previousMoonRadius + location.radius + toOrbit.radius;
				previousMoonRadius = distance + location.radius - toOrbit.radius;

				totalMoonOffset++;
				moonOffset++;
			} else {
				distance = minPlanetSpacing + previousPlanetRadius + location.radius;
				previousPlanetRadius = distance + location.radius;
				orbitCenter = starCenter;

				previousMoonRadius = 0.0f;
				moonOffset = 0;
			}

			location.orbit.angle += location.orbit.speed * delta / distance;
			location.orbit.angle = fmod(location.orbit.angle, M_PI * 2);
			
			// Orbit path
			{
				UICircleData orbitPath = {};
				orbitPath.strokeColor = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
				orbitPath.strokeStyle = UICircleStrokeStyle::solid;
				orbitPath.radius = distance;
				orbitPath.thickness = 1.0f;
				orbitPath.position = orbitCenter;
				gameState->uiElements.push(orbitPath);
			}

			// Location
			{
				UICircleData circle = {};
				circle.color = location.color;
				circle.radius = location.radius;
				circle.position = Vec2(1.0f, 0.0f);

				const f32 ca = cos(0.0f);
				const f32 sa = sin(0.0f);
				circle.position = Vec2(
					ca * circle.position.x - sa * circle.position.y, 
					sa * circle.position.x + ca * circle.position.y
				);

				circle.position = circle.position * distance + orbitCenter;
				location.position = circle.position; // Position used for orbiting moons

				const f32 inputDistance = gameState->input.mouse.distanceTo(circle.position);
				const bool mouseIsOver = inputDistance <= location.radius + minMoonSpacing * 0.5f;
				if (mouseIsOver) {
					circle.thickness = 1.0f;
					circle.strokeColor = Rgba(0.0f, 1.0f, 0.0f, 1.0f);
					locationHighlighted = true;
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

		gameState->input.cursor = locationHighlighted ? Cursor::pointer : Cursor::arrow;
	}
};