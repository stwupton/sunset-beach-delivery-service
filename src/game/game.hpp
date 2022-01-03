#pragma once

#include <cmath>

#include "common/game_state.hpp"
#include "game/combat.hpp"
#include "game/system_view.hpp"

namespace Game {
	void setup(GameState *gameState) {
		gameState->modes[(size_t)GameModeId::combat] = { &Combat::setup, &Combat::update };
		gameState->modes[(size_t)GameModeId::systemView] = { &SystemView::setup, &SystemView::update };

		gameState->modes[(size_t)gameState->mode].setup(gameState);
	}

	void update(GameState *gameState, f32 delta) {
		if (gameState->pendingMode != GameModeId::none) {
			gameState->modes[(size_t)gameState->pendingMode].setup(gameState);

			gameState->mode = gameState->pendingMode;
			gameState->pendingMode = GameModeId::none;

			return;
		}

		gameState->modes[(size_t)gameState->mode].update(gameState, delta);
	}
};