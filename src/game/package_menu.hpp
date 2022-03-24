#pragma once

#include <cmath>

#include "common/game_state.hpp"
#include "game/utils.hpp"
#include "types/core.hpp"

namespace PackageMenu {

	// Forward declerations
	void setup(GameState *gameState);
	void update(GameState *gameState, f32 delta);
	void drawBackground(GameState *gameState);
	void drawUI(GameState *gameState);

	template<typename T, size_t Size>
	void addSprites(SpriteBuffer *sprites, Array<T, Size> toRender) {
		for (const T &sprite : toRender) {
			sprites->push((Sprite)sprite);
		}
	}

	void setup(GameState *gameState) {
		gameState->textureLoadQueue.push(TextureAssetId::marketPlace1);

		/*Shipment shipment1 = Shipment{};
		srand(gameState->daysPassed);
		shipment1.creditAward = rand() * ;

		gameState->systemLocations.length*/

		gameState->updateSystems.clear();
		gameState->updateSystems.push(&update);
	}

	void update(GameState *gameState, f32 delta) {
		drawBackground(gameState);
		drawUI(gameState);
	}

	void drawTitle(GameState *gameState) {
		UITextData titleText = {};
		swprintf_s(titleText.text.data, L"%s", gameState->dockedLocation->name);
		//titleText.text = gameState->selectedLocation->name;
		titleText.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
		titleText.font = L"consolas";
		titleText.fontSize = 40.0f;
		titleText.width = 200.0f;
		titleText.height = 30.0f;
		f32 xPos = (screenWidth / 2.0f) - titleText.fontSize - 20.0f;
		titleText.position = Vec2(xPos, 20.0f);
		titleText.horizontalAlignment = UITextAlignment::start;
		titleText.verticalAlignment = UITextAlignment::middle;
		gameState->uiElements.push(titleText);
	}

	void drawPackages(GameState *gameState) {

		for (size_t i = 0; i < gameState->availableShipments.length; i++) {

			UITextData packageText = {};
			swprintf_s(packageText.text.data, L"Package %d", i + 1);
			packageText.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
			packageText.font = L"consolas";
			packageText.fontSize = 40.0f;
			packageText.width = 200.0f;
			packageText.height = 30.0f;
			f32 xPos = (screenWidth / 5.0f) - (packageText.width / 2.0f);
			f32 yPos = (screenHeight / 2.0f) - 20.0f;
			packageText.position = Vec2(xPos, yPos);
			packageText.horizontalAlignment = UITextAlignment::start;
			packageText.verticalAlignment = UITextAlignment::middle;
			gameState->uiElements.push(packageText);

			UITextData weightText = {};
			swprintf_s(weightText.text.data, L"Weight: %f", gameState->availableShipments[i].weight);
			weightText.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
			weightText.font = L"consolas";
			weightText.fontSize = 20.0f;
			weightText.width = 200.0f;
			weightText.height = 30.0f;
			yPos += packageText.height + 20.0f;;
			weightText.position = Vec2(xPos, yPos);
			weightText.horizontalAlignment = UITextAlignment::start;
			weightText.verticalAlignment = UITextAlignment::middle;
			gameState->uiElements.push(weightText);

			UITextData destinationText = {};
			swprintf_s(destinationText.text.data, L"Destination: %s", gameState->availableShipments[i].to->name);
			destinationText.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
			destinationText.font = L"consolas";
			destinationText.fontSize = 20.0f;
			destinationText.width = 250.0f;
			destinationText.height = 30.0f;
			yPos += weightText.height;
			destinationText.position = Vec2(xPos, yPos);
			destinationText.horizontalAlignment = UITextAlignment::start;
			destinationText.verticalAlignment = UITextAlignment::middle;
			gameState->uiElements.push(destinationText);

			UITextData creditText = {};
			swprintf_s(creditText.text.data, L"Value: %d", gameState->availableShipments[i].creditAward);
			creditText.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
			creditText.font = L"consolas";
			creditText.fontSize = 20.0f;
			creditText.width = 200.0f;
			creditText.height = 30.0f;
			yPos += destinationText.height;
			creditText.position = Vec2(xPos, yPos);
			creditText.horizontalAlignment = UITextAlignment::start;
			creditText.verticalAlignment = UITextAlignment::middle;
			gameState->uiElements.push(creditText);

			const bool enabled = gameState->availableShipments[i].available;
			const f32 alpha = enabled ? 1.0f : 0.4f;

			UIButtonData button = {};
			button.label.text = L"SELECT";
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
			button.enabled = enabled;
			yPos += creditText.height;
			button.position = Vec2(xPos, yPos);

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

						gameState->shipments.push(gameState->availableShipments[i]);
						gameState->availableShipments[i].available = false;
					}
				}
			}
			gameState->uiElements.push(button);
		}
	}

	void drawPackage2(GameState *gameState) {
		UIRectangleData packageImage = {};
		packageImage.color = Rgba(1.0f, 1.0f, 1.0f, 0.0f);
		packageImage.height = 500.0f;
		packageImage.width = 200.0f;
		f32 xPos = ((screenWidth / 5.0f) * 2.0f) - (packageImage.width / 2.0f);
		f32 yPos = (screenHeight / 2.0f);
		yPos -= packageImage.height / 2.0f;
		packageImage.position = Vec2(xPos, yPos);
		packageImage.strokeWidth = 5.0f;
		packageImage.strokeColor = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
		//packageImage.fillRectange = false;

		gameState->uiElements.push(packageImage);

		UITextData packageText = {};
		packageText.text = L"Package 2";
		packageText.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
		packageText.font = L"consolas";
		packageText.fontSize = 40.0f;
		packageText.width = 200.0f;
		packageText.height = 30.0f;
		yPos += packageImage.height + packageText.height;
		packageText.position = Vec2(xPos, yPos);
		packageText.horizontalAlignment = UITextAlignment::start;
		packageText.verticalAlignment = UITextAlignment::middle;
		gameState->uiElements.push(packageText);
	}

	void drawPackage3(GameState *gameState) {
		UIRectangleData packageImage = {};
		packageImage.color = Rgba(1.0f, 1.0f, 1.0f, 0.0f);
		packageImage.height = 500.0f;
		packageImage.width = 200.0f;
		f32 xPos = ((screenWidth / 5.0f) * 3.0f) - (packageImage.width / 2.0f);
		f32 yPos = (screenHeight / 2.0f);
		yPos -= packageImage.height / 2.0f;
		packageImage.position = Vec2(xPos, yPos);
		packageImage.strokeWidth = 5.0f;
		packageImage.strokeColor = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
		//packageImage.fillRectange = false;

		gameState->uiElements.push(packageImage);

		UITextData packageText = {};
		packageText.text = L"Package 3";
		packageText.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
		packageText.font = L"consolas";
		packageText.fontSize = 40.0f;
		packageText.width = 200.0f;
		packageText.height = 30.0f;
		yPos += packageImage.height + packageText.height;
		packageText.position = Vec2(xPos, yPos);
		packageText.horizontalAlignment = UITextAlignment::start;
		packageText.verticalAlignment = UITextAlignment::middle;
		gameState->uiElements.push(packageText);
	}

	void drawPackage4(GameState *gameState) {
		UIRectangleData packageImage = {};
		packageImage.color = Rgba(1.0f, 1.0f, 1.0f, 0.0f);
		packageImage.height = 500.0f;
		packageImage.width = 200.0f;
		f32 xPos = ((screenWidth / 5.0f) * 4.0f) - (packageImage.width / 2.0f);
		f32 yPos = (screenHeight / 2.0f);
		yPos -= packageImage.height / 2.0f;
		packageImage.position = Vec2(xPos, yPos);
		packageImage.strokeWidth = 5.0f;
		packageImage.strokeColor = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
		//packageImage.fillRectange = false;

		gameState->uiElements.push(packageImage);

		UITextData packageText = {};
		packageText.text = L"Package 4";
		packageText.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
		packageText.font = L"consolas";
		packageText.fontSize = 40.0f;
		packageText.width = 200.0f;
		packageText.height = 30.0f;
		yPos += packageImage.height + packageText.height;
		packageText.position = Vec2(xPos, yPos);
		packageText.horizontalAlignment = UITextAlignment::start;
		packageText.verticalAlignment = UITextAlignment::middle;
		gameState->uiElements.push(packageText);
	}

	void drawExitButton(GameState *gameState) {

		const f32 alpha = 1.0f;
		const f32 bottomPadding = 20.0f;

		UIButtonData button = {};
		button.label.text = L"EXIT";
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
		f32 xPos = (screenWidth / 2.0f) - button.label.fontSize - 20.0f;
		f32 yPos = screenHeight - button.height - bottomPadding;
		button.position = Vec2(xPos, yPos);

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

				SystemSelect::setup(gameState);
			}

		}
		gameState->uiElements.push(button);
	}


	void drawBackground(GameState *gameState) {
		Sprite background = {};
		background.assetId = TextureAssetId::marketPlace1;
		background.position = Vec3<f32>(0.0f, 0.0f, 0.9f);
		background.scale = Vec2<f32>(1.3f, 1.3f);
		gameState->sprites.push(background);
	}

	void drawUI(GameState *gameState) {
		drawTitle(gameState);
		drawPackages(gameState);
		drawPackage2(gameState);
		drawPackage3(gameState);
		drawPackage4(gameState);
		drawExitButton(gameState);
	}
};