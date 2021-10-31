#pragma once

#include "common/sprite.hpp"
#include "common/ui_element.hpp"
#include "types/core.hpp"

// TODO(steven): We're in game now, we shouldn't be directly communicating with 
// platform layer systems. Come up with a nice way to send and receive references
// to loaded texture information.
#include "platform/windows/dx3d_sprite_loader.cpp"

class Game {
protected:
	// TODO(steven): Cache these in the sprite loader instead.
	Dx3dSpriteResource shipResource;
	Dx3dSpriteResource enemyShipResource;
	Dx3dSpriteResource backgroundResource;

	Sprite background;
	Sprite enemyShip;
	Sprite ship;
	
public:
	void load(Dx3dSpriteLoader *loader) {
		this->shipResource = loader->load(AssetId::ship);
		this->enemyShipResource = loader->load(AssetId::enemyShip);
		this->backgroundResource = loader->load(AssetId::background);
	}

	void setup() {
		this->ship.textureReference = (void*)&this->shipResource;
		this->ship.position = Vec3<f32>(0.0f, -300.0f, 0.4f);
		this->ship.scale = Vec2<f32>(0.5f, 0.5f);
		this->ship.angle = -95.0f;

		this->enemyShip.textureReference = (void*)&this->enemyShipResource;
		this->enemyShip.position = Vec3<f32>(0.0f, 300.0f, 0.4f);
		this->enemyShip.scale = Vec2<f32>(0.5f, 0.5f);
		this->enemyShip.angle = -95.0f;

		this->background.textureReference = (void*)&this->backgroundResource;
		this->background.position = Vec3<f32>(0.0f, 0.0f, 0.9f);
		this->background.scale = Vec2<f32>(1.3f, 1.3f);
	}

	void update(GameState *gameState, f32 delta) {
		SpriteBuffer &sprites = gameState->sprites;
		sprites.clear();
		sprites.push(this->background);
		sprites.push(this->ship);
		sprites.push(this->enemyShip);

		UIElementBuffer &uiElements = gameState->uiElements;
		uiElements.clear();

		WCHAR textBuffer[100] = {};

		u8 fps = 1 / delta;
		swprintf_s(textBuffer, L"FPS: %d", fps);
		UITextData fpsCount = {};
		fpsCount.text = textBuffer;
		fpsCount.fontSize = 30.0f;
		fpsCount.position = 0;
		fpsCount.width = 300.0f;
		fpsCount.height = 40.0f;
		uiElements.pushText(fpsCount);

		swprintf_s(textBuffer, L"Mouse Down: %d", gameState->input.primaryButton.down);
		UITextData buttonMessage = {};
		buttonMessage.text = textBuffer;
		buttonMessage.fontSize = 30.0f;
		buttonMessage.position = Vec2<f32>(0.0f, 40.0f);
		buttonMessage.width = 300.0f;
		buttonMessage.height = 40.0f;
		uiElements.pushText(buttonMessage);

		if (gameState->input.primaryButton.down) {
			UILineData drawLine = {};
			drawLine.start = gameState->input.primaryButton.start;
			drawLine.end = gameState->input.mouse;
			drawLine.thickness = 30.0f;
			uiElements.pushLine(drawLine);

			swprintf_s(textBuffer, L"Button is pressed");
			UITextData debugText = {};
			debugText.text = textBuffer;
			debugText.fontSize = 20.0f;
			debugText.position = Vec2<f32>(250.0f, 80.0f);
			debugText.width = 250.0f;
			debugText.height = 40.0f;
			uiElements.pushText(debugText);
		}
	}
};