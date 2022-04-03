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
	void drawPackageOptions(GameState *gameState);
	void deliverPackages(GameState *gameState);
	
	enum class PackageMenuState {
		main,
		pickup,
		dropoff
	};
	PackageMenuState mode = PackageMenuState::main;
	bool hasAvailablePackages = false;

	void setup(GameState *gameState) {
		gameState->textureLoadQueue.push(TextureAssetId::marketPlace1);

		gameState->updateSystems.clear();
		gameState->updateSystems.push(&update);

		mode = PackageMenuState::main;

		hasAvailablePackages = false;
		for (size_t i = 0; i < gameState->shipments.length; i++) {
			// only show packages for current location
			if (gameState->shipments[i].to == gameState->dockedLocation && gameState->shipments[i].available) {
				hasAvailablePackages = true;
				break;
			}
		}
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
		f32 xPos = (screenWidth / 2.0f) - (titleText.width / 2.0f); //titleText.fontSize - 20.0f;
		titleText.position = Vec2(xPos, 20.0f);
		titleText.horizontalAlignment = UITextAlignment::start;
		titleText.verticalAlignment = UITextAlignment::middle;
		gameState->uiElements.push(titleText);
	}

	void drawPackageDropoffs(GameState *gameState) {

		const f32 packageWidth = 200.0f;
		const f32 packagePadding = 80.0f;

		// Shipments for docked location
		Array<int, 10> deliverableShipments;

		for (size_t i = 0; i < gameState->shipments.length; i++) {
			// only show packages for current location
			if (gameState->shipments[i].to == gameState->dockedLocation) {
				deliverableShipments.push(i);
			}
		}

		int numOfPackages = deliverableShipments.length;

		// Calculate total width of all packages with padding
		f32 totalWidth = (numOfPackages * packageWidth) + ((numOfPackages - 1) * packagePadding);
		f32 midPoint = (screenWidth / 2.0f);
		f32 startXPos = midPoint - (totalWidth / 2.0f);

		//int divisor = numOfPackages == 1 ? 2 : numOfPackages;
		//f32 startingPoint = (screenWidth / (double)(divisor));
		//f32 startXPos = startingPoint - ((packagePadding / 2.0f) * (numOfPackages - 1)) - packageWidth;// (packageWidth / 2.0f);
		f32 previousXPosDiff = 0.0f;

		for (size_t i = 0; i < deliverableShipments.length; i++) {
			// only show packages for current location
			if (gameState->shipments[deliverableShipments[i]].to == gameState->dockedLocation) {
				f32 xPos = startXPos + previousXPosDiff;
				f32 yPos = (screenHeight / 2.0f) - 20.0f;

				UITextData packageText = {};
				swprintf_s(packageText.text.data, L"Package %d", deliverableShipments[i] + 1);
				packageText.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
				packageText.font = L"consolas";
				packageText.fontSize = 40.0f;
				packageText.width = 200.0f;
				packageText.height = 30.0f;
				packageText.position = Vec2(xPos, yPos);
				packageText.horizontalAlignment = UITextAlignment::start;
				packageText.verticalAlignment = UITextAlignment::middle;
				gameState->uiElements.push(packageText);

				UITextData weightText = {};
				swprintf_s(weightText.text.data, L"Weight: %f", gameState->shipments[deliverableShipments[i]].weight);
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
				swprintf_s(destinationText.text.data, L"Destination: %s", gameState->shipments[deliverableShipments[i]].to->name);
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
				swprintf_s(creditText.text.data, L"Value: %d", gameState->shipments[deliverableShipments[i]].creditAward);
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

				const bool enabled = gameState->shipments[deliverableShipments[i]].available;
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

				previousXPosDiff += packageWidth + packagePadding;

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

							gameState->credits += gameState->shipments[deliverableShipments[i]].creditAward;
							// Play cash sound
							gameState->soundLoadQueue.push(SoundAssetId::cha_ching);
							gameState->soundLoadQueue.push(SoundAssetId::wahoo);
							gameState->shipments[deliverableShipments[i]].available = false;
						}
					}
				}
				gameState->uiElements.push(button);
			}
		}

		// Probably can do this once per setup
		deliverableShipments.clear();
	}		

	void drawPackages(GameState *gameState) {

		const f32 packageWidth = 200.0f;
		const f32 packagePadding = 80.0f;
		int numOfPackages = gameState->availableShipments.length;

		// Calculate total width of all packages with padding
		f32 totalWidth = (numOfPackages * packageWidth) + ((numOfPackages - 1) * packagePadding);
		f32 midPoint = (screenWidth / 2.0f);
		f32 startXPos = midPoint - (totalWidth / 2.0f);

		//int divisor = numOfPackages == 1 ? 2 : numOfPackages;
		//f32 startingPoint = (screenWidth / (double)(divisor));
		//f32 startXPos = startingPoint - ((packagePadding / 2.0f) * (numOfPackages - 1)) - packageWidth;// (packageWidth / 2.0f);
		f32 previousXPosDiff = 0.0f;

		for (size_t i = 0; i < gameState->availableShipments.length; i++) {

			f32 xPos = startXPos + previousXPosDiff;
			f32 yPos = (screenHeight / 2.0f) - 20.0f;

			UITextData packageText = {};
			swprintf_s(packageText.text.data, L"Package %d", i + 1);
			packageText.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
			packageText.font = L"consolas";
			packageText.fontSize = 40.0f;
			packageText.width = 200.0f;
			packageText.height = 30.0f;
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

			previousXPosDiff += packageWidth + packagePadding;

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
		//f32 xPos = (screenWidth / 2.0f) - button.label.fontSize - 20.0f;
		f32 xPos = (screenWidth / 2.0f) - (button.width / 2.0f);
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

	void drawReturnButton(GameState *gameState) {

		const f32 alpha = 1.0f;
		const f32 bottomPadding = 20.0f;

		UIButtonData button = {};
		button.label.text = L"RETURN";
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
		//f32 xPos = (screenWidth / 2.0f) - button.label.fontSize - 20.0f;
		f32 xPos = (screenWidth / 2.0f) - (button.width / 2.0f);
		f32 yPos = screenHeight - button.height - bottomPadding;
		button.position = Vec2(xPos, yPos);

		button.handleInput(gameState->input);
		if (button.checkInput(UIButtonInputState::over)) {
			gameState->input.cursor = Cursor::pointer;
			button.strokeColor = Rgba(0.0f, 1.0f, 0.0f, 1.0f);

			if (button.checkInput(UIButtonInputState::clicked)) {
				const f32 scale = 0.9f;
				button.label.fontSize *= scale;
				button.position += Vec2(
					button.width - button.width * scale,
					button.height - button.height * scale
				) * 0.5f;
				button.width *= scale;
				button.height *= scale;
				button.strokeWidth *= scale;

				mode = PackageMenuState::main;
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
		switch (mode)
		{
			case PackageMenu::PackageMenuState::main:
				drawPackageOptions(gameState);
				drawExitButton(gameState);
				break;
			case PackageMenu::PackageMenuState::pickup:
				drawPackages(gameState);
				drawReturnButton(gameState);
				break;
			case PackageMenu::PackageMenuState::dropoff:
				drawPackageDropoffs(gameState);
				drawReturnButton(gameState);
				break;
			default:
				break;
		}

		/*drawPackage2(gameState);
		drawPackage3(gameState);
		drawPackage4(gameState);*/
	}

	void drawPackageOptions(GameState *gameState) {
		f32 alpha = 1.0f;

		const f32 buttonWidth = 200.0f;
		const f32 buttonPadding = 80.0f;
		int numOfButtons = 2;
		f32 previousXPosDiff = 0.0f;

		// Calculate total width of all packages with padding
		f32 totalWidth = (numOfButtons * buttonWidth) + ((numOfButtons - 1) * buttonPadding);
		f32 midPoint = (screenWidth / 2.0f);
		f32 startXPos = midPoint - (totalWidth / 2.0f);


		f32 xPos = startXPos + previousXPosDiff;

		UIButtonData btnPickup = {};
		btnPickup.label.text = L"PICKUP";
		btnPickup.label.font = L"consolas";
		btnPickup.label.color = Rgba(0.0f, 1.0f, 0.0f, 1.0f);
		btnPickup.label.fontSize = 24.0f;
		btnPickup.label.color = Rgba(1.0f, 1.0f, 1.0f, alpha);
		btnPickup.color = Rgba(0.0f, 0.0f, 0.0f, alpha);
		btnPickup.height = 70.0f;
		btnPickup.width = buttonWidth;
		btnPickup.cornerRadius = 10.0f;
		btnPickup.strokeColor = Rgba(0.62f, 0.62f, 0.62f, 1.0f);
		btnPickup.strokeWidth = 5.0f;
		//f32 xPos = (screenWidth / 2.0f) - (btnPickup.width / 2.0f);
		f32 yPos = (screenHeight / 2.0f) - (btnPickup.height / 2.0f);
		btnPickup.position = Vec2(xPos, yPos);

		btnPickup.handleInput(gameState->input);
		if (btnPickup.checkInput(UIButtonInputState::over)) {
			gameState->input.cursor = Cursor::pointer;
			btnPickup.strokeColor = Rgba(0.0f, 1.0f, 0.0f, 1.0f);

			if (btnPickup.checkInput(UIButtonInputState::down)) {
				const f32 scale = 0.9f;
				btnPickup.label.fontSize *= scale;
				btnPickup.position += Vec2(
					btnPickup.width - btnPickup.width * scale,
					btnPickup.height - btnPickup.height * scale
				) * 0.5f;
				btnPickup.width *= scale;
				btnPickup.height *= scale;
				btnPickup.strokeWidth *= scale;

				mode = PackageMenuState::pickup;
			}

		}
		gameState->uiElements.push(btnPickup);


		previousXPosDiff += buttonWidth + buttonPadding;
		xPos = startXPos + previousXPosDiff;

		const bool enabled = hasAvailablePackages;
		alpha = enabled ? 1.0f : 0.4f;
		UIButtonData btnDropoff = {};
		btnDropoff.label.text = L"DROP OFF";
		btnDropoff.label.font = L"consolas";
		btnDropoff.label.color = Rgba(0.0f, 1.0f, 0.0f, 1.0f);
		btnDropoff.label.fontSize = 24.0f;
		btnDropoff.label.color = Rgba(1.0f, 1.0f, 1.0f, alpha);
		btnDropoff.color = Rgba(0.0f, 0.0f, 0.0f, alpha);
		btnDropoff.height = 70.0f;
		btnDropoff.width = 150.0f;
		btnDropoff.cornerRadius = 10.0f;
		btnDropoff.strokeColor = Rgba(0.62f, 0.62f, 0.62f, 1.0f);
		btnDropoff.strokeWidth = 5.0f;
		//f32 xPos = (screenWidth / 2.0f) - (btnDropoff.width / 2.0f);
		//f32 yPos = (screenHeight / 2.0f) - (btnDropoff.height / 2.0f);
		btnDropoff.position = Vec2(xPos, yPos);
		btnDropoff.enabled = enabled;

		if (btnDropoff.enabled) {
			btnDropoff.handleInput(gameState->input);
			if (btnDropoff.checkInput(UIButtonInputState::over)) {
				gameState->input.cursor = Cursor::pointer;
				btnDropoff.strokeColor = Rgba(0.0f, 1.0f, 0.0f, 1.0f);

				if (btnDropoff.checkInput(UIButtonInputState::down)) {
					const f32 scale = 0.9f;
					btnDropoff.label.fontSize *= scale;
					btnDropoff.position += Vec2(
						btnDropoff.width - btnDropoff.width * scale,
						btnDropoff.height - btnDropoff.height * scale
					) * 0.5f;
					btnDropoff.width *= scale;
					btnDropoff.height *= scale;
					btnDropoff.strokeWidth *= scale;

					//mode = PackageMenuState::dropoff;
					deliverPackages(gameState);
				}

			}
		}
		gameState->uiElements.push(btnDropoff);
	}

	void deliverPackages(GameState *gameState)
	{
		for (size_t i = 0; i < gameState->shipments.length; i++) {
			// only show packages for current location
			if (gameState->shipments[i].to == gameState->dockedLocation) {
				gameState->credits += gameState->shipments[i].creditAward;
				// Play cash sound
				gameState->shipments[i].available = false;
			}
		}

		hasAvailablePackages = false;

		// TODO: Display "toast" of how much money (moo-la) has been made

		gameState->soundLoadQueue.push(SoundAssetId::cha_ching);
		gameState->soundLoadQueue.push(SoundAssetId::wahoo); // TODO: Add delay so this gets played 100 milliseconds after cha_ching
	}
};