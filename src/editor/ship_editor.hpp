#pragma once

#include "common/game_state.hpp"
#include "editor/utils.hpp"

namespace ShipEditor {
	void setup(GameState *gameState) {
		gameState->editorState.shipEditorState.ship = gameState->templates.ship;
	}

	void rotationUpdate(ShipEditorState *state, Input input) {
		// TODO(steven): hide and lock cursor to the center

		state->ship.angle += input.mouseMovement().x;
		if (input.primaryButton.wasDown && !input.primaryButton.down) {
			state->mode = ShipEditorMode::none;
		}
	}

	void scaleUpdate(ShipEditorState *state, Input input) {
		const f32 scaleBy = input.mouseMovement().x;
		state->ship.scale += Vec2(scaleBy, scaleBy) * 0.01f;

		if (input.primaryButton.wasDown && !input.primaryButton.down) {
			state->mode = ShipEditorMode::none;
		}
	}

	void update(GameState *gameState) {
		ShipEditorState &state = gameState->editorState.shipEditorState;

		Sprite background = {};
		background.assetId = TextureAssetId::background;
		background.position = Vec3<f32>(0.0f, 0.0f, 0.9f);
		background.scale = Vec2<f32>(1.3f, 1.3f);
		gameState->sprites.push(background);

		gameState->sprites.push(state.ship);

		if (state.mode != ShipEditorMode::none) {
			switch (state.mode) {
				case ShipEditorMode::rotate: {
					rotationUpdate(&state, gameState->input); 
				} break;

				case ShipEditorMode::scale: {
					scaleUpdate(&state, gameState->input); 
				} break;
			}

			return;
		}

		UIButtonData rotButton = uiButton(L"Rotate", Vec2(100.0f, 100.0f), gameState->input);
		gameState->uiElements.push(rotButton);
		if (rotButton.checkInput(UIButtonInputState::clicked)) {
			state.mode = ShipEditorMode::rotate;
		}

		UIButtonData scaleButton = uiButton(L"Scale", Vec2(100.0f, 250.0f), gameState->input);
		gameState->uiElements.push(scaleButton);
		if (scaleButton.checkInput(UIButtonInputState::clicked)) {
			state.mode = ShipEditorMode::scale;
		}

		UIButtonData saveButton = uiButton(L"💾", Vec2(100.0f, 880.0f), gameState->input);
		gameState->uiElements.push(saveButton);
		if (saveButton.checkInput(UIButtonInputState::clicked)) {
			SaveData &saveData = gameState->editorState.saveData;

			saveData.pending = true;
			saveData.path = L"assets/data/templates/ships/ship";
			// TODO(steven): Obviously dumping the binary data to file is not an ideal 
			// solution but it works ok for now. Do proper serialisation.
			memcpy(saveData.buffer, (void*)&state.ship, sizeof(Ship));
			saveData.size = sizeof(Ship);

			gameState->templates.ship = state.ship;
		}
	}
};