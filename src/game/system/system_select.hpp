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
	void drawIndicator(GameState *gameState);
	void drawUI(GameState *gameState);
	void highlightLocations(GameState *gameState);

	const f32 starRadius = 400.0f;
	const Vec2<f32> starCenter = Vec3(-250.0f, 1080.0f * 0.5f);
	const f32 minPlanetSpacing = 100.0f;
	const f32 minMoonSpacing = minPlanetSpacing * 0.2f;
	const FuelValue fuelBurnRate = 20;

	void setup(GameState *gameState) {
		gameState->textureLoadQueue.push(TextureAssetId::background);
	}

	void update(GameState *gameState, f32 delta) {
		gameState->highlightedLocation = nullptr;

		if (gameState->input.keyDown == '\t') {
			gameState->nextMode = GameModeId::systemView;
		}

		if (gameState->targetLocation != nullptr) {
			// TODO(steven): Change this formula for when we are using the position 
			// around orbit indtead of system select view position.
			const f32 travelDistance = gameState->dockedLocation->position
				.distanceTo(gameState->targetLocation->position);
			gameState->travelProgress += delta / travelDistance;
			gameState->travelProgress = min(1.0f, gameState->travelProgress);

			// Update fuel
			gameState->playerShip.fuel = 
				gameState->fuelBeforeTravel - 
				gameState->travelFuelConsumption * 
				gameState->travelProgress;

			// Once we have reached target location
			if (gameState->travelProgress == 1.0f) {
				gameState->dockedLocation = gameState->targetLocation;
				gameState->targetLocation = nullptr;
				gameState->travelProgress = 0.0f;
			}
		}

		SystemCommon::drawStarField(gameState);
		SystemCommon::drawCentralStar(gameState, starCenter, starRadius);
		drawLocations(gameState, delta);
		drawIndicator(gameState);
		highlightLocations(gameState);
		drawUI(gameState);
	}

	void drawIndicator(GameState *gameState) {
		const SystemLocation *startLocation = gameState->dockedLocation;
		const SystemLocation *targetLocation = gameState->targetLocation;
		if (startLocation != nullptr) {
			UITriangleData indicator = {};
			indicator.color = Rgba(0.16f, 0.94f, 0.9f, 1.0f);

			indicator.points.push(startLocation->position + Vec2(0.0f, -startLocation->radius - 10.0f));
			indicator.points.push(startLocation->position + Vec2(-10.0f, -startLocation->radius - 30.0f));
			indicator.points.push(startLocation->position + Vec2(10.0f, -startLocation->radius - 30.0f));

			gameState->uiElements.push(indicator);
		}
	}

	void drawLocations(GameState *gameState, f32 delta) {
		u8 totalMoonOffset = 0;
		u8 moonOffset = 0;

		f32 previousPlanetRadius = starRadius;
		f32 previousMoonRadius = 0.0f;

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
				const f32 allowedInputArea = location.radius + minMoonSpacing * 0.5f;
				const bool mouseIsOver = inputDistance <= allowedInputArea;
				if (mouseIsOver) {
					gameState->highlightedLocation = &location;
				}

				gameState->uiElements.push(circle);

				if (mouseIsOver) {
					if (
						gameState->input.primaryButton.wasDown && 
						gameState->input.primaryButton.start.distanceTo(circle.position) <= allowedInputArea
					) {
						gameState->selectedLocation = &location;
					}

					UITextData locationLabel = {};
					locationLabel.text = location.name.data;
					locationLabel.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
					locationLabel.font = L"consolas";
					locationLabel.fontSize = 24.0f;
					locationLabel.width = 200.0f;
					locationLabel.height = locationLabel.fontSize;
					locationLabel.position = 
						circle.position + 
						Vec2(-locationLabel.width * 0.5f, circle.radius + 10.0f);
					locationLabel.horizontalAlignment = UITextAlignment::middle;
					gameState->uiElements.push(locationLabel);
				}
			}
		}
	}

	void drawUI(GameState *gameState) {
		// Fuel gauge
		{
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

			SystemLocation *targetLocation = nullptr;
			if (gameState->selectedLocation != nullptr) {
				targetLocation = gameState->selectedLocation;
			}

			if (gameState->highlightedLocation != nullptr) {
				targetLocation = gameState->highlightedLocation;
			}

			if (targetLocation != nullptr) {
				FuelValue fuelConsumption = 
					gameState->dockedLocation->position.distanceTo(targetLocation->position) / 
					fuelBurnRate;

				if (gameState->playerShip.fuel >= fuelConsumption) {
					f32 fuelConsumptionScale = fuelConsumption / gameState->playerShip.fuelTankCapacity;
					fuelConsumptionScale = min(1.0f, max(0.0f, fuelConsumptionScale));

					UILineData fuelConsumptionVisual = fuelGauge;
					fuelConsumptionVisual.color = Rgba(1.0f, 0.0f, 0.0f, 1.0f);
					fuelConsumptionVisual.start = fuelGauge.end;
					fuelConsumptionVisual.end = fuelConsumptionVisual.start - Vec2(200.0f) * fuelConsumptionScale;
					fuelConsumptionVisual.thickness = 30.0f;
					gameState->uiElements.push(fuelConsumptionVisual);
				}
			}
		}

		if (gameState->selectedLocation != nullptr) {
			FuelValue fuelConsumption = 
				gameState->dockedLocation->position.distanceTo(gameState->selectedLocation->position) / 
				fuelBurnRate;
			
			// TODO(steven): Maybe make the button disable instead of disappear.
			if (gameState->playerShip.fuel >= fuelConsumption) {
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

						gameState->targetLocation = gameState->selectedLocation;
						gameState->selectedLocation = nullptr;
						gameState->travelProgress = 0.0f;
						gameState->travelFuelConsumption = fuelConsumption;
						gameState->fuelBeforeTravel = gameState->playerShip.fuel;
					}
				}

				gameState->uiElements.push(button);
			}
		}
	}

	void highlightLocations(GameState *gameState) {
		gameState->input.cursor = Cursor::arrow;

		if (gameState->selectedLocation != nullptr) {
			UICircleData highlight = {};
			highlight.thickness = 2.0f;
			highlight.strokeColor = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
			highlight.position = gameState->selectedLocation->position;
			highlight.radius = gameState->selectedLocation->radius;
			gameState->uiElements.push(highlight);
		}

		if (gameState->highlightedLocation != nullptr) {
			UICircleData highlight = {};
			highlight.thickness = 2.0f;
			highlight.strokeColor = Rgba(0.0f, 1.0f, 0.0f, 1.0f);
			highlight.position = gameState->highlightedLocation->position;
			highlight.radius = gameState->highlightedLocation->radius;
			gameState->uiElements.push(highlight);

			gameState->input.cursor = Cursor::pointer;
		}
	}
};