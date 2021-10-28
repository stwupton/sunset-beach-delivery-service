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
		UIElement fpsCount = {};
		fpsCount.type = UIType::text;
		fpsCount.text.text = textBuffer;
		fpsCount.text.fontSize = 30.0f;
		fpsCount.text.position = 0;
		fpsCount.text.width = 300.0f;
		fpsCount.text.height = 40.0f;
		uiElements.push(fpsCount);

		swprintf_s(textBuffer, L"Mouse Down: %d", gameState->input.primaryButton.down);
		UIElement buttonMessage = {};
		buttonMessage.type = UIType::text;
		buttonMessage.text.text = textBuffer;
		buttonMessage.text.fontSize = 30.0f;
		buttonMessage.text.position = Vec2<f32>(0.0f, 40.0f);
		buttonMessage.text.width = 300.0f;
		buttonMessage.text.height = 40.0f;
		uiElements.push(buttonMessage);

		if (gameState->input.primaryButton.down) {
			UIElement drawLine = {};
			drawLine.type = UIType::line;
			drawLine.line.start = gameState->input.primaryButton.start;
			drawLine.line.end = gameState->input.mouse;
			drawLine.line.thickness = 30.0f;
			uiElements.push(drawLine);
		}
	}
};