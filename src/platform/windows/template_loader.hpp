#pragma once

#include "common/game_state.hpp"
#include "platform/windows/file_loader.hpp"

void loadTemplates(GameState *gameState) {
	ShipTemplates &shipTemplates = gameState->templates.ships;
	shipTemplates.push({ L"Ally Ship", L"ships/ship" });
	shipTemplates.push({ L"Enemy Ship", L"ships/enemy_ship" });

	wchar_t filePathBuffer[100];
	for (TemplateData<Ship> &shipTemplate : shipTemplates) {
		swprintf_s(filePathBuffer, L"assets/data/templates/%s", shipTemplate.fileName.data);
		load(filePathBuffer, &shipTemplate.data, sizeof(Ship));
	}
}