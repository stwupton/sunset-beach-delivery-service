#pragma once

#include "common/sprite.hpp"
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

	void update(Sprite **spriteBuffer, u8 *spriteLength, f32 delta) {
		*spriteBuffer = &this->background;
		*spriteLength = 3;
	}
};