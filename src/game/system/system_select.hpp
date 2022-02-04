#pragma once

#include <cmath>

#include "common/game_state.hpp"
#include "game/utils.hpp"
#include "game/system/common.hpp"
#include "types/core.hpp"
#include "types/vector.hpp"

namespace SystemSelect {
	// Forward declerations
	void drawLocations(GameState *gameState, f32 delta);
	void drawUI(GameState *gameState);

	const f32 starRadius = 400.0f;
	const Vec2<f32> starCenter = Vec3(-250.0f, 1080.0f * 0.5f);
	const f32 minPlanetSpacing = 100.0f;
	const f32 minMoonSpacing = minPlanetSpacing * 0.2f;

	void setup(GameState *gameState) {
		gameState->textureLoadQueue.push(TextureAssetId::background);
	}

	void update(GameState *gameState, f32 delta) {
		SystemCommon::drawStarField(gameState);

		if (gameState->input.keyDown == '\t') {
			gameState->nextMode = GameModeId::systemView;
		}

		SystemCommon::drawCentralStar(gameState, starCenter, starRadius);
		drawLocations(gameState, delta);
		drawUI(gameState);
	}

	void drawLocations(GameState *gameState, f32 delta) {
		Array<UICircleData, 6> orbitPaths;
		Array<UICircleData, 6> locations;
		UITriangleData dockedIndicator = {};

		UITextData locationLabel = {};
		bool showLabel = false;

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

			SystemCommon::updateOrbitLocation(&location, delta);
			
			// Orbit path
			{
				UICircleData orbitPath = {};
				orbitPath.strokeColor = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
				orbitPath.strokeStyle = UICircleStrokeStyle::solid;
				orbitPath.radius = distance;
				orbitPath.thickness = 1.0f;
				orbitPath.position = orbitCenter;
				orbitPaths.push(orbitPath);
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
				const f32 allowedInputArea = location.radius + minMoonSpacing * 0.5f;
				const bool mouseIsOver = inputDistance <= allowedInputArea;
				if (mouseIsOver || gameState->selectedLocation == &location) {
					circle.thickness = 1.0f;
					circle.strokeColor = Rgba(0.0f, 1.0f, 0.0f, 1.0f);
					locationHighlighted = true;
				}

				locations.push(circle);

				if (mouseIsOver) {
					if (
						gameState->input.primaryButton.wasDown && 
						gameState->input.primaryButton.start.distanceTo(circle.position) <= allowedInputArea
					) {
						gameState->selectedLocation = &location;
					}

					locationLabel.text = location.name.data;
					locationLabel.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
					locationLabel.fontSize = 30.0f;
					locationLabel.position = circle.position;
					locationLabel.height = 30.0f;
					locationLabel.width = 200.0f;
					showLabel = true;
				}

				if (gameState->dockedLocation == &location) {
					dockedIndicator.color = Rgba(0.16f, 0.94f, 0.9f, 1.0f);

					dockedIndicator.points.push(circle.position + Vec2(0.0f, -circle.radius - 10.0f));
					dockedIndicator.points.push(circle.position + Vec2(-10.0f, -circle.radius - 30.0f));
					dockedIndicator.points.push(circle.position + Vec2(10.0f, -circle.radius - 30.0f));
				}
			}
		}

		for (const UICircleData &orbit : orbitPaths) {
			gameState->uiElements.push(orbit);
		}

		for (const UICircleData &location : locations) {
			gameState->uiElements.push(location);
		}

		gameState->uiElements.push(dockedIndicator);

		if (showLabel) {
			gameState->uiElements.push(locationLabel);
		}

		gameState->input.cursor = locationHighlighted ? Cursor::pointer : Cursor::arrow;
	}

	void drawUI(GameState *gameState) {
		UITextData fuelLabel = {};
		fuelLabel.text = L"FUEL: ";
		fuelLabel.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
		fuelLabel.font = L"consolas";
		fuelLabel.fontSize = 20.0f;
		fuelLabel.position = Vec2(1920.0f - 400.0f, 1080.0f - fuelLabel.fontSize - 20.0f);
		fuelLabel.width = 100.0f;
		fuelLabel.height = 30.0f;
		fuelLabel.horizontalAlignment = UITextAlignment::end;
		fuelLabel.verticalAlignment = UITextAlignment::middle;
		gameState->uiElements.push(fuelLabel);

		UILineData fuelGaugeBackground = {};
		fuelGaugeBackground.color = Rgba(0.5f, 0.5f, 0.5f, 1.0f);
		fuelGaugeBackground.start = Vec2(
			fuelLabel.position.x + fuelLabel.width, 
			fuelLabel.position.y + fuelLabel.height * 0.5f
		);
		fuelGaugeBackground.end = fuelGaugeBackground.start + Vec2(200.0f);
		fuelGaugeBackground.thickness = 30.0f;
		gameState->uiElements.push(fuelGaugeBackground);

		f32 fuelPercent = (f32)gameState->playerShip.fuel / gameState->playerShip.fuelTankCapacity;
		fuelPercent = min(1.0f, max(0.0f, fuelPercent));

		UILineData fuelGauge = fuelGaugeBackground;
		fuelGauge.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
		fuelGauge.end = fuelGauge.start + Vec2(200.0f) * fuelPercent;
		fuelGauge.thickness = 30.0f;
		gameState->uiElements.push(fuelGauge);

		if (gameState->selectedLocation != nullptr) {
			f32 fuelConsumptionPercent = 
				gameState->dockedLocation->position.distanceTo(gameState->selectedLocation->position) / 
				20.0f / 
				gameState->playerShip.fuelTankCapacity;
			fuelConsumptionPercent = min(1.0f, max(0.0f, fuelConsumptionPercent));

			UILineData fuelConsumption = fuelGauge;
			fuelConsumption.color = Rgba(1.0f, 0.0f, 0.0f, 1.0f);
			fuelConsumption.start = fuelGauge.end;
			fuelConsumption.end = fuelConsumption.end - Vec2(200.0f) * fuelConsumptionPercent;
			fuelConsumption.thickness = 30.0f;
			gameState->uiElements.push(fuelConsumption);

			UIButtonData button = {};
			button.label.text = L"DEPART";
			button.label.fontSize = 30.0f;
			button.label.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
			button.color = Rgba(0.3f, 0.3f, 0.3f, 1.0f);
			button.height = 100.0f;
			button.width = 200.0f;
			button.position = Vec2(1920.0f - button.width - 10.0f, 10.0f);

			button.handleInput(gameState->input);
			if (button.checkInput(UIButtonInputState::over)) {
				gameState->input.cursor = Cursor::pointer;
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

			gameState->uiElements.push(button);
		}
	}
};