#pragma once

#include "common/asset_definitions.hpp"
#include "common/game_state.hpp"
#include "common/sprite.hpp"
#include "common/window_config.hpp"
#include "common/ui_element.hpp"
#include "types/core.hpp"
#include "common/window_config.hpp"

class Game {
private:
	f32 currentLineLength = 0.0f;
protected:
	Sprite background;
	Sprite enemyShip;
	Sprite ship;

public:
	SoundManager* man;

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
			gameState->shipTargets.push(
				{ CombatParty::ally, 100, 100, Vec2<f32>(-300.0f, -200.0f) }
			);
			gameState->shipTargets.push(
				{ CombatParty::enemy, 200, 100, Vec2<f32>(0.0f, 300.0f) }
			);
		}

		// Weapons
		{
			gameState->weapons.push(
				{ CombatParty::ally, Vec2<f32>(60.0f, -300.0f) }
			);
			gameState->weapons.push(
				{ CombatParty::enemy, Vec2<f32>(-60.0f, -300.0f) }
			);
		}
	}

	void update(GameState *gameState, f32 delta) {
		SpriteBuffer &sprites = gameState->sprites;
		sprites.clear();
		sprites.push(this->background);
		sprites.push(this->ship);
		sprites.push(this->enemyShip);

		gameState->uiElements.clear();
		this->updateDebugUI(gameState, delta);
		this->handleUserTargeting(gameState);
	}

protected:
	void handleUserTargeting(GameState *gameState) const {
		UIElementBuffer &uiElements = gameState->uiElements;

		bool isTargeting = false;	
		Vec2<f32> targetingWeaponPosition;
		for (u8 i = 0; i < gameState->weapons.length; i++) {
			const Weapon &weapon = gameState->weapons[i];

			UICircleData weaponIndicator = {};
			// TODO(steven): We're converting 3D posiiton to 2D positions here, depending 
			// on how often we do this, it might be worth making it a util function
			weaponIndicator.position = Vec2<f32>(
				screenWidth * 0.5f + weapon.position.x,
				screenHeight * 0.5f - weapon.position.y
			);
			weaponIndicator.thickness = 10.0f;
			weaponIndicator.radius = 50.0f;

			if (weapon.party == CombatParty::ally) {
				weaponIndicator.color = Rgba(0.0f, 1.0f, 0.0f, 1.0f);
			} else {
				weaponIndicator.color = Rgba(1.0f, 0.0f, 0.0f, 1.0f);
			}

			uiElements.push(weaponIndicator);

			if (weapon.party == CombatParty::ally && gameState->input.primaryButton.down) {
				const f32 difference = gameState->input.primaryButton.start.distanceTo(weaponIndicator.position);
				if (difference < weaponIndicator.radius) {
					targetingWeaponPosition = weaponIndicator.position;
					isTargeting = true;
				}
			}
		}

		// Draw combat ship targets
		// TODO(steven): Just using debug circles for now, represent them another way
		bool hasLockOn = false;
		Vec2<f32> lockOnPosition;
		for (u8 i = 0; i < gameState->shipTargets.length; i++) {
			const ShipTarget &target = gameState->shipTargets[i];

			UICircleData targetPoint = {};
			// TODO(steven): We're converting 3D posiiton to 2D positions here, depending 
			// on how often we do this, it might be worth making it a util function
			targetPoint.position = Vec2<f32>(
				screenWidth * 0.5f + target.position.x,
				screenHeight * 0.5f - target.position.y
			);
			targetPoint.thickness = 10.0f;
			targetPoint.radius = 50.0f;
			targetPoint.style = UICircleStyle::dotted;

			if (target.party == CombatParty::ally) {
				targetPoint.color = Rgba(0.0f, 1.0f, 0.0f, 1.0f);
			} else {
				targetPoint.color = Rgba(1.0f, 0.0f, 0.0f, 1.0f);
			}

			uiElements.push(targetPoint);

			if (target.party == CombatParty::enemy && gameState->input.primaryButton.down) {
				const f32 difference = gameState->input.mouse.distanceTo(targetPoint.position);
				if (difference < targetPoint.radius) {
					lockOnPosition = targetPoint.position;
					hasLockOn = true;
				}
			}

			UILineData healthBar = {};
			const f32 healthScale = (f32)target.health / target.maxHealth;
			healthBar.start = targetPoint.position + Vec2<f32>(-50.0f, -100.0f);
			healthBar.end = healthBar.start + Vec2<f32>(100.0f * healthScale, 0.0f);
			healthBar.color = Rgba(abs(healthScale - 1.0f), healthScale, 0.0f, 1.0f);
			healthBar.thickness = 20.0f;
			uiElements.push(healthBar);
		}

		if (isTargeting) {
			UILineData drawLine = {};
			drawLine.start = targetingWeaponPosition;

			if (hasLockOn) {
				drawLine.end = lockOnPosition;
			} else {
				drawLine.end = gameState->input.mouse;
			}

			drawLine.thickness = 30.0f;
			drawLine.color = Rgba(1.0f, 0.0f, 0.0f, 1.0f);
			uiElements.push(drawLine);
		}
	}

	void updateDebugUI(GameState *gameState, f32 delta) const {
		UIElementBuffer &uiElements = gameState->uiElements;

		wchar_t textBuffer[100] = {};

		u8 fps = round(1 / delta);
		swprintf_s(textBuffer, L"FPS: %d", fps);
		UITextData fpsCount = {};
		fpsCount.text = textBuffer;
		fpsCount.fontSize = 30.0f;
		fpsCount.position = 0;
		fpsCount.width = 300.0f;
		fpsCount.height = 40.0f;
		fpsCount.color = Rgba(1.0f, 0.0f, 0.0f, 1.0f);
		uiElements.push(fpsCount);

		swprintf_s(textBuffer, L"Mouse Down: %d", gameState->input.primaryButton.down);
		UITextData buttonMessage = {};
		buttonMessage.text = textBuffer;
		buttonMessage.fontSize = 30.0f;
		buttonMessage.position = Vec2<f32>(0.0f, 40.0f);
		buttonMessage.width = 300.0f;
		buttonMessage.height = 40.0f;
		buttonMessage.color = Rgba(1.0f, 0.0f, 0.0f, 1.0f);
		uiElements.push(buttonMessage);

		const Vec2<f32> mouse = gameState->input.mouse;
		swprintf_s(textBuffer, L"X: %d, Y: %d", (u32)mouse.x, (u32)mouse.y);
		UITextData mouseCoords = {};
		mouseCoords.text = textBuffer;
		mouseCoords.fontSize = 30.0f;
		mouseCoords.position = Vec2<f32>(0.0f, 80.0f);
		mouseCoords.width = 300.0f;
		mouseCoords.height = 40.0f;
		mouseCoords.color = Rgba(1.0f, 0.0f, 0.0f, 1.0f);
		uiElements.push(mouseCoords);

		// We want to normalise and clamp between two values...
		f32 pitch = gameState->input.mouse.x / screenWidth;
		pitch = max(0.0f, min(1.0f, pitch));
		man->SetPitch(pitch);

		UITextData debugText = {};
		swprintf_s(textBuffer, L"Pitch: %f", pitch);
		debugText.text = textBuffer;
		debugText.fontSize = 20.0f;
		debugText.position = Vec2<f32>(250.0f, 120.0f);
		debugText.width = 250.0f;
		debugText.height = 40.0f;
		debugText.color = Rgba(1.0f, 0.0f, 0.0f, 1.0f);
		uiElements.push(debugText);
	}
};