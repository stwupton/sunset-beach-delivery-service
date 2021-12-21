#pragma once 

#include "common/game_state.hpp"
#include "common/editor_state.hpp"

namespace Menu {
	void update(GameState *gameState) {
		UIButtonData shipEditorButton = {};
		shipEditorButton.label.text = L"Ship Editor";
		shipEditorButton.label.fontSize = 30.0f;
		shipEditorButton.label.color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
		shipEditorButton.color = Rgba(0.4f, 0.4f, 0.4f, 1.0f);
		shipEditorButton.height = 100.0f;
		shipEditorButton.width = 200.0f;
		shipEditorButton.position = Vec2(100.0f, 100.0f);
		gameState->uiElements.push(shipEditorButton);

		if (shipEditorButton.clicked(gameState->input.primaryButton)) {
			gameState->editorState.mode = EditorMode::ship;
		}
	}
};