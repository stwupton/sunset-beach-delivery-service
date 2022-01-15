#pragma once

#include "common/save_data.hpp"
#include "common/ship.hpp"
#include "common/templates.hpp"

enum class ShipEditorMode {
	none,
	rotate,
	scale
};

struct ShipEditorState {
	TemplateData<Ship> *shipTemplate;
	ShipEditorMode mode = ShipEditorMode::none;
};

enum class EditorMode {
	menu,
	ship
};

struct EditorState {
	EditorMode mode = EditorMode::menu;
	ShipEditorState shipEditorState;
	SaveData saveData;
};