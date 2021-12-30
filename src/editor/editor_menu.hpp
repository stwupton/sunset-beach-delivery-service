#pragma once 

#include "common/editor_state.hpp"
#include "common/game_state.hpp"
#include "editor/ship_editor.hpp"
#include "editor/utils.hpp"

namespace EditorMenu {
	void update(GameState *gameState) {
		UI_BUTTON(shipEditorButton, L"Ship Editor", Vec2(100.0f, 100.0f))
		gameState->uiElements.push(shipEditorButton);

		if (shipEditorButton.clicked(gameState->input.primaryButton)) {
			ShipEditor::setup(gameState);
			gameState->editorState.mode = EditorMode::ship;
		}
	}
};