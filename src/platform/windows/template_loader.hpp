#pragma once

#include "common/game_state.hpp"
#include "platform/windows/file_loader.hpp"

void loadTemplates(GameState *gameState) {
	load(L"assets/data/templates/ships/ship", &gameState->templates.ship, sizeof(Ship));
}