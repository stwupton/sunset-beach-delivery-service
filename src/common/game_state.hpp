#pragma once

#include <cmath>

#include "common/editor_state.hpp"
#include "common/asset_definitions.hpp"
#include "common/event.hpp"
#include "common/sprite.hpp"
#include "common/input.hpp"
#include "common/load_queue.hpp"
#include "common/projectile.hpp"
#include "common/ship.hpp"
#include "common/ship_target.hpp"
#include "common/shipment.hpp"
#include "common/system_location.hpp"
#include "common/templates.hpp"
#include "common/ui_element_buffer.hpp"
#include "common/weapon.hpp"
#include "types/array.hpp"

typedef Array<Sprite, 10> SpriteBuffer;
typedef LoadQueue<TextureAssetId, 8> TextureLoadQueue;
typedef LoadQueue<SoundAssetId, 8> SoundLoadQueue;

typedef void (*GameModeSetup)(struct GameState *gameState);
typedef void (*GameModeUpdate)(struct GameState *gameState, f32 delta);

struct GameMode {
	GameModeSetup setup;
	GameModeUpdate update;
};

enum class GameModeId : u8 {
	none,
	systemSelect,
	systemView,
	combat,
	_length
};

struct GameState {
	// Combat data
	Array<AimlessProjectile, 100> aimlessProjectiles;
	Ship playerShip;
	Array<Ship, 2> allyShips;
	Array<Ship, 2> enemyShips;
	Array<Projectile, 100> projectiles;

	// System view data
	Array<SystemLocation, 6> systemLocations;
	SystemLocation *selectedLocation = nullptr;
	SystemLocation *highlightedLocation = nullptr;
	SystemLocation *targetLocation = nullptr;
	SystemLocation *dockedLocation = nullptr;

	// Journey data
	struct {
		FuelValue fuelConsumption = 0.0f;
		FuelValue fuelBefore = 0.0f;
		f32 progress = 0.0f;
	} journey;

	// Delivery data
	u32 deliveriesMade = 0;
	Array<Shipment, 10> shipments;

	// Platform/game common data
	Events events;
	Input input;
	GameModeId mode = GameModeId::systemSelect;
	GameMode modes[(size_t)GameModeId::_length];
	GameModeId nextMode;
	SpriteBuffer sprites;
	Templates templates;
	TextureLoadQueue textureLoadQueue;
	SoundLoadQueue soundLoadQueue;
	MusicAssetId pendingMusicItem = MusicAssetId::none;
	UIElementBuffer uiElements;

#ifdef DEBUG
	EditorState editorState;
	u8 gameSpeed = 1;
#endif
};