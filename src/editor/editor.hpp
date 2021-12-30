#pragma once 

#include "common/game_state.hpp"
#include "types/core.hpp"
#include "editor/editor_menu.hpp"
#include "editor/ship_editor.hpp"

namespace Editor {
	void update(GameState *gameState) {
		string16<32> modeNames[] = { L"EditorMenu", L"Ship" };
		string16<32> modeName = modeNames[(size_t)gameState->editorState.mode];

		string16<64> text;
		swprintf_s(text.data, L"Mode: %s", modeName.data);

		UITextData modeText = {};
		modeText.text = text.data;
		modeText.color = Rgba(0.0f, 1.0f, 0.0f, 1.0f);
		modeText.fontSize = 24.0f;
		modeText.width = 200.0f;
		modeText.height = 100.0f;
		modeText.position = Vec2(1920.0f - 200.0f, 100.0f);
		gameState->uiElements.push(modeText);
		
		switch (gameState->editorState.mode) {
			case EditorMode::menu: {
				EditorMenu::update(gameState);
			} break;

			case EditorMode::ship: {
				ShipEditor::update(gameState);
			} break;
		}
	}
};