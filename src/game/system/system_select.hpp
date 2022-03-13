#pragma once

#include <cmath>

#include "common/game_state.hpp"
#include "game/date.hpp"
#include "game/utils.hpp"
#include "game/system/common.hpp"
#include "game/system/system_view.hpp"
#include "types/core.hpp"
#include "types/vector.hpp"

namespace SystemSelect {
	// Forward declerations
	void drawLocations(GameState *gameState, f32 delta);
	void drawIndicator(GameState *gameState);
	void drawUI(GameState *gameState);
	f32 getDistanceFromStar(SystemLocation *location);
	void highlightLocations(GameState *gameState);
	void update(GameState *gameState, f32 delta);
	void updateJourney(GameState *gameState, f32 delta);

	const f32 starRadius = 400.0f;
	const Vec2<f32> starCenter = Vec3(-250.0f, 1080.0f * 0.5f);
	const f32 minPlanetSpacing = 130.0f;
	const f32 minMoonSpacing = minPlanetSpacing * 0.2f;
	const FuelValue fuelBurnRate = 20;
	const f32 dayRate = 0.01f;

	void setup(GameState *gameState) {
		gameState->textureLoadQueue.push(TextureAssetId::background);
		
		gameState->updateSystems.clear();
		gameState->updateSystems.push(&update);
	}

	void update(GameState *gameState, f32 delta) {
		gameState->highlightedLocation = nullptr;

		if (gameState->input.keyDown == '\t') {
			SystemView::setup(gameState);
		}

		updateJourney(gameState, delta);
		SystemCommon::drawStarField(gameState);
		SystemCommon::drawCentralStar(gameState, starCenter, starRadius);
		drawLocations(gameState, delta);
		drawIndicator(gameState);
		highlightLocations(gameState);
		drawUI(gameState);
	}

	void drawCredits(GameState *gameState) {
		UITextData creditText = {};
		swprintf_s(creditText.text.data, L"CREDITS: %c%u", 0xa4, gameState->credits);
		creditText.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
		creditText.font = L"consolas";
		creditText.fontSize = 20.0f;
		creditText.width = 200.0f;
		creditText.height = 30.0f;
		creditText.position = Vec2(
			(1920.0f - creditText.width) * 0.5f, 
			1080.0f - creditText.fontSize - 20.0f
		);
		creditText.horizontalAlignment = UITextAlignment::middle;
		creditText.verticalAlignment = UITextAlignment::middle;
		gameState->uiElements.push(creditText);
	}

	void drawDate(GameState *gameState) {
		UITextData dateText = {};
		dateText.text = DateUtils::getDate(gameState->daysPassed).data;
		dateText.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
		dateText.font = L"consolas";
		dateText.fontSize = 20.0f;
		dateText.width = 200.0f;
		dateText.height = 30.0f;
		dateText.position = Vec2(20.0f, 1080.0f - dateText.fontSize - 20.0f);
		dateText.horizontalAlignment = UITextAlignment::start;
		dateText.verticalAlignment = UITextAlignment::middle;
		gameState->uiElements.push(dateText);
	}

	void drawDepartButton(GameState *gameState) {
		if (gameState->selectedLocation == gameState->dockedLocation) {
			return;
		}

		const f32 distance = gameState->dockedLocation->position
			.distanceTo(gameState->selectedLocation->position);
		FuelValue fuelConsumption = distance / fuelBurnRate;
		
		const bool enabled = 
			gameState->selectedLocation != gameState->dockedLocation && 
			gameState->playerShip.fuel >= fuelConsumption;
		const f32 alpha = enabled ? 1.0f : 0.4f;

		UIButtonData button = {};
		button.label.text = L"DEPART";
		button.label.font = L"consolas";
		button.label.color = Rgba(0.0f, 1.0f, 0.0f, 1.0f);
		button.label.fontSize = 24.0f;
		button.label.color = Rgba(1.0f, 1.0f, 1.0f, alpha);
		button.color = Rgba(0.0f, 0.0f, 0.0f, alpha);
		button.height = 70.0f;
		button.width = 150.0f;
		button.cornerRadius = 10.0f;
		button.strokeColor = Rgba(0.62f, 0.62f, 0.62f, 1.0f);
		button.strokeWidth = 5.0f;
		button.position = Vec2(1920.0f - button.width - 10.0f, 10.0f);

		const f32 travelDistance = abs(
			getDistanceFromStar(gameState->dockedLocation) - 
			getDistanceFromStar(gameState->selectedLocation)
		);
		const DayValue estimatedDays = max(1, round(travelDistance * dayRate));
		UITextData estimatedDaysText = {};
		swprintf_s(estimatedDaysText.text.data, L"ESTIMATED TRAVEL TIME: %d DAYS", estimatedDays);
		estimatedDaysText.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
		estimatedDaysText.font = L"consolas";
		estimatedDaysText.fontSize = 14.0f;
		estimatedDaysText.width = button.width;
		estimatedDaysText.height = estimatedDaysText.fontSize * 2;
		estimatedDaysText.horizontalAlignment = UITextAlignment::middle;
		estimatedDaysText.verticalAlignment = UITextAlignment::middle;
		estimatedDaysText.position = Vec2(button.position.x, button.position.y + button.height + 20.0f);

		if (enabled) {
			button.handleInput(gameState->input);
			if (button.checkInput(UIButtonInputState::over)) {
				gameState->input.cursor = Cursor::pointer;
				button.strokeColor = Rgba(0.0f, 1.0f, 0.0f, 1.0f);

				if (button.checkInput(UIButtonInputState::down)) {
					const f32 scale = 0.9f;
					button.label.fontSize *= scale;
					button.position += Vec2(
						button.width - button.width * scale, 
						button.height - button.height * scale
					) * 0.5f;
					button.width *= scale;
					button.height *= scale;
					button.strokeWidth *= scale;

					Tween journeyTween = {};
					journeyTween.duration = estimatedDays;
					journeyTween.type = TweenValueType::float32;
					*(f32*)journeyTween.from = 0.0f;
					*(f32*)journeyTween.to = 1.0f;
					journeyTween.value = &gameState->journeyProgress;
					gameState->tweens.push(journeyTween);

					Tween fuelTween = {};
					fuelTween.duration = estimatedDays;
					fuelTween.type = TweenValueType::float32;
					*(f32*)fuelTween.from = gameState->playerShip.fuel;
					*(f32*)fuelTween.to = gameState->playerShip.fuel - fuelConsumption;
					fuelTween.value = &gameState->playerShip.fuel;
					gameState->tweens.push(fuelTween);

					Tween daysTween = {};
					daysTween.duration = estimatedDays;
					daysTween.type = TweenValueType::int32;
					*(s32*)daysTween.from = gameState->daysPassed;
					*(s32*)daysTween.to = gameState->daysPassed + estimatedDays;
					daysTween.value = &gameState->daysPassed;
					gameState->tweens.push(daysTween);

					gameState->targetLocation = gameState->selectedLocation;
					gameState->selectedLocation = nullptr;
				}
			}
		} else {
			estimatedDaysText.text = L"INSUFFICIENT FUEL";
		}

		gameState->uiElements.push(button);
		gameState->uiElements.push(estimatedDaysText);
	}

	void drawFuelGauge(GameState *gameState) {
		// Label
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

		// Background
		UILineData fuelGaugeBackground = {};
		fuelGaugeBackground.color = Rgba(0.5f, 0.5f, 0.5f, 1.0f);
		fuelGaugeBackground.start = Vec2(
			fuelLabel.position.x + fuelLabel.width, 
			fuelLabel.position.y + fuelLabel.height * 0.5f
		);
		fuelGaugeBackground.end = fuelGaugeBackground.start + Vec2(200.0f);
		fuelGaugeBackground.thickness = 30.0f;
		gameState->uiElements.push(fuelGaugeBackground);

		// Current fuel
		f32 fuelPercent = (f32)gameState->playerShip.fuel / gameState->playerShip.fuelTankCapacity;
		fuelPercent = min(1.0f, max(0.0f, fuelPercent));

		UILineData fuelGauge = fuelGaugeBackground;
		fuelGauge.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
		fuelGauge.end = fuelGauge.start + Vec2(200.0f) * fuelPercent;
		fuelGauge.thickness = 30.0f;
		gameState->uiElements.push(fuelGauge);

		// Estamated journey consumption
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

			f32 fuelConsumptionScale = fuelConsumption / gameState->playerShip.fuelTankCapacity;
			fuelConsumptionScale = min(1.0f, max(0.0f, fuelConsumptionScale));

			UILineData fuelConsumptionVisual = fuelGauge;
			fuelConsumptionVisual.color = Rgba(1.0f, 0.0f, 0.0f, 1.0f);
			fuelConsumptionVisual.start = fuelGauge.end;
			fuelConsumptionVisual.end = fuelConsumptionVisual.start - Vec2(200.0f) * fuelConsumptionScale;
			fuelConsumptionVisual.end.x = max(fuelGauge.start.x, fuelConsumptionVisual.end.x);
			fuelConsumptionVisual.thickness = 30.0f;
			gameState->uiElements.push(fuelConsumptionVisual);
		}
	}

	void drawIndicator(GameState *gameState) {
		const SystemLocation *startLocation = gameState->dockedLocation;
		const SystemLocation *targetLocation = gameState->targetLocation;
		if (startLocation != nullptr) {
			UITriangleData indicator = {};
			indicator.color = Rgba(0.16f, 0.94f, 0.9f, 1.0f);

			Vec2<f32> offset;
			if (targetLocation != nullptr) {
				offset.y = -(100 - startLocation->radius);

				const Vec2<f32> difference = targetLocation->position - startLocation->position;
				offset.x = difference.x * gameState->journeyProgress;
			}

			// Bottom, top-left & top-right
			indicator.points.push(startLocation->position + Vec2(0.0f, -startLocation->radius - 10.0f) + offset);
			indicator.points.push(startLocation->position + Vec2(-10.0f, -startLocation->radius - 30.0f) + offset);
			indicator.points.push(startLocation->position + Vec2(10.0f, -startLocation->radius - 30.0f) + offset);

			gameState->uiElements.push(indicator);
		}
	}

	void drawLocations(GameState *gameState, f32 delta) {
		const f32 orbitDistanceScale = 0.07f;

		u8 moonOffset = 0;

		f32 previousLocationDistance = starRadius;
		f32 previousMoonDistance = 0.0f;

		for (size_t i = 0; i < gameState->systemLocations.length; i++) {
			Vec2<f32> orbitCenter = Vec2<f32>();
			f32 distance = 0.0f;

			SystemLocation &location = gameState->systemLocations[i];

			if (i != 0 && location.isMoon) {
				const SystemLocation &toOrbit = gameState->systemLocations[i - moonOffset - 1];
				orbitCenter = toOrbit.position;
				distance = 
					minMoonSpacing + 
					previousMoonDistance + 
					toOrbit.radius + 
					location.orbit.distance * 
					orbitDistanceScale;
				previousMoonDistance = distance - toOrbit.radius;

				moonOffset++;
			} else {
				distance = minPlanetSpacing + previousLocationDistance + location.orbit.distance * orbitDistanceScale;
				previousLocationDistance = distance;
				orbitCenter = starCenter;

				previousMoonDistance = 0.0f;
				moonOffset = 0;
			}

			SystemCommon::updateOrbitLocation(&location, delta);
			
			// Orbit path
			{
				UICircleData orbitPath = {};
				orbitPath.strokeColor = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
				orbitPath.strokeStyle = UIStrokeStyle::solid;
				orbitPath.radius = distance;
				orbitPath.strokeWidth = 1.0f;
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
		drawFuelGauge(gameState);
		drawCredits(gameState);
		drawDate(gameState);
		if (gameState->selectedLocation != nullptr) {
			drawDepartButton(gameState);
		}
	}

	f32 getDistanceFromStar(SystemLocation *location) {
		while (location->isMoon) {
			location--;
		}
		return location->orbit.distance;
	}

	void highlightLocations(GameState *gameState) {
		gameState->input.cursor = Cursor::arrow;

		if (gameState->selectedLocation != nullptr) {
			UICircleData highlight = {};
			highlight.strokeWidth = 2.0f;
			highlight.strokeColor = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
			highlight.position = gameState->selectedLocation->position;
			highlight.radius = gameState->selectedLocation->radius;
			gameState->uiElements.push(highlight);
		}

		if (gameState->highlightedLocation != nullptr) {
			UICircleData highlight = {};
			highlight.strokeWidth = 2.0f;
			highlight.strokeColor = Rgba(0.0f, 1.0f, 0.0f, 1.0f);
			highlight.position = gameState->highlightedLocation->position;
			highlight.radius = gameState->highlightedLocation->radius;
			gameState->uiElements.push(highlight);

			gameState->input.cursor = Cursor::pointer;
		}
	}

	void updateJourney(GameState *gameState, f32 delta) {
		if (gameState->targetLocation != nullptr && gameState->journeyProgress == 1.0f) {
			gameState->dockedLocation = gameState->targetLocation;
			gameState->targetLocation = nullptr;
			gameState->journeyProgress = 0.0f;
		}
	}
};