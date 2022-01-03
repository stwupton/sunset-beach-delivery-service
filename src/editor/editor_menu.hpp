#pragma once 

#include "common/editor_state.hpp"
#include "common/game_state.hpp"
#include "editor/ship_editor.hpp"
#include "editor/utils.hpp"

namespace EditorMenu {
	void update(GameState *gameState) {
		UIButtonData shipEditorButton = uiButton(L"Ship Editor", Vec2(100.0f, 100.0f), &gameState->input);
		gameState->uiElements.push(shipEditorButton);

		if (shipEditorButton.checkInput(UIButtonInputState::clicked)) {
			ShipEditor::setup(gameState);
			gameState->editorState.mode = EditorMode::ship;
		}
	}
};