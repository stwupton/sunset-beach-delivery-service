#pragma once

#include "common/asset_definitions.hpp"
#include "common/sprite.hpp"
#include "common/ui_element.hpp"
#include "types/core.hpp"

class Game {
protected:
	Sprite background;
	Sprite enemyShip;
	Sprite ship;
	
public:
	void load(GameState *gameState) {
		gameState->loadQueue.push(TextureAssetId::ship);
		gameState->loadQueue.push(TextureAssetId::enemyShip);
		gameState->loadQueue.push(TextureAssetId::background);
	}

	void setup(GameState *gameState) {
		// Sprites
		{
			this->ship.assetId = TextureAssetId::ship;
			this->ship.position = Vec3<f32>(0.0f, -300.0f, 0.4f);
			this->ship.scale = Vec2<f32>(0.5f, 0.5f);
			this->ship.angle = -95.0f;

			this->enemyShip.assetId = TextureAssetId::enemyShip;
			this->enemyShip.position = Vec3<f32>(0.0f, 300.0f, 0.4f);
			this->enemyShip.scale = Vec2<f32>(0.2f, 0.2f);
			this->enemyShip.angle = -180.0f;

			this->background.assetId = TextureAssetId::background;
			this->background.position = Vec3<f32>(0.0f, 0.0f, 0.9f);
			this->background.scale = Vec2<f32>(1.3f, 1.3f);
		}

		// Ship Targets
		{
			ShipTarget target = {};
			target.maxHealth = target.health = 200;
			target.party = CombatParty::enemy;
			target.position = Vec2<f32>(0.0f, 300.0f);
			gameState->shipTargets.push(target);
		}
	}

	void update(GameState *gameState, f32 delta) {
		SpriteBuffer &sprites = gameState->sprites;
		sprites.clear();
		sprites.push(this->background);
		sprites.push(this->ship);
		sprites.push(this->enemyShip);

		UIElementBuffer &uiElements = gameState->uiElements;
		uiElements.clear();

		wchar_t textBuffer[100] = {};

		u8 fps = round(1 / delta);
		swprintf_s(textBuffer, L"FPS: %d", fps);
		UITextData fpsCount = {};
		fpsCount.text = textBuffer;
		fpsCount.fontSize = 30.0f;
		fpsCount.position = 0;
		fpsCount.width = 300.0f;
		fpsCount.height = 40.0f;
		uiElements.push(fpsCount);

		swprintf_s(textBuffer, L"Mouse Down: %d", gameState->input.primaryButton.down);
		UITextData buttonMessage = {};
		buttonMessage.text = textBuffer;
		buttonMessage.fontSize = 30.0f;
		buttonMessage.position = Vec2<f32>(0.0f, 40.0f);
		buttonMessage.width = 300.0f;
		buttonMessage.height = 40.0f;
		uiElements.push(buttonMessage);

		const Vec2<f32> mouse = gameState->input.mouse;
		swprintf_s(textBuffer, L"X: %d, Y: %d", (u32)mouse.x, (u32)mouse.y);
		UITextData mouseCoords = {};
		mouseCoords.text = textBuffer;
		mouseCoords.fontSize = 30.0f;
		mouseCoords.position = Vec2<f32>(0.0f, 80.0f);
		mouseCoords.width = 300.0f;
		mouseCoords.height = 40.0f;
		uiElements.push(mouseCoords);

		if (gameState->input.primaryButton.down) {
			UILineData drawLine = {};
			drawLine.start = gameState->input.primaryButton.start;
			drawLine.end = gameState->input.mouse;
			drawLine.thickness = 30.0f;
			uiElements.push(drawLine);
		}

		// Draw combat ship targets
		// TODO(steven): Just using debug circles for now, represent them another way
		for (u8 i = 0; i < gameState->shipTargets.length; i++) {
			const ShipTarget &target = gameState->shipTargets[i];

			UICircleData targetPoint = {};
			// TODO(steven): We're converting 3D posiiton to 2D positions here, depending 
			// on how often we do this, it might be worth making it a util function
			targetPoint.position = Vec2<f32>(
				screenWidth * 0.5f + target.position.x,
				screenHeight * 0.5f - target.position.y
			);
			targetPoint.radius = 50.0f;
			uiElements.push(targetPoint);

			// TODO(steven): Make health bar from lines, will probably require a different
			// colour other than red
		}
	}
};