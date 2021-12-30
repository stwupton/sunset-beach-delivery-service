#pragma once

#include "common/save_data.hpp"
#include "common/ship.hpp"

enum class EditorMode {
	menu,
	ship
};

enum class ShipEditorMode {
	none,
	rotate,
	scale
};

struct ShipEditorState {
	Ship ship;
	ShipEditorMode mode = ShipEditorMode::none;
};

struct EditorState {
	EditorMode mode = EditorMode::menu;
	ShipEditorState shipEditorState;
	SaveData saveData;
};