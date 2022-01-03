#pragma once 

#include "common/game_state.hpp"
#include "types/core.hpp"
#include "editor/editor_menu.hpp"
#include "editor/ship_editor.hpp"

namespace Editor {
	void update(GameState *gameState) {
		String16<32> modeNames[] = { L"EditorMenu", L"ShipEditor" };
		String16<32> modeName = modeNames[(size_t)gameState->editorState.mode];

		String16<64> text;
		swprintf_s(text.data, L"Mode: %s", modeName.data);

		// Mode display
		UITextData debugText = {};
		debugText.text = text.data;
		debugText.color = Rgba(0.0f, 1.0f, 0.0f, 1.0f);
		debugText.fontSize = 24.0f;
		debugText.width = 400.0f;
		debugText.height = 100.0f;
		debugText.position = Vec2(1920.0f - 400.0f, 100.0f);
		gameState->uiElements.push(debugText);

		debugText.text = L"[E] Enter/exit editor";
		debugText.position.y += 50.0f;
		gameState->uiElements.push(debugText);

		if (gameState->editorState.mode == EditorMode::menu) {
			EditorMenu::update(gameState);
		} else {
			swprintf_s(text.data, L"[Q] Quit %s", modeName.data);
			debugText.text = text.data;
			debugText.position.y += 50.0f;
			gameState->uiElements.push(debugText);

			if (gameState->input.keyDown == 'Q') {
				gameState->editorState.mode = EditorMode::menu;
			}

			switch (gameState->editorState.mode) {
				case EditorMode::ship:
					ShipEditor::update(gameState);
					break;
			}
		}
	}
};