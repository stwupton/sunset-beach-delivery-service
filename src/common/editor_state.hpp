#pragma once

enum class EditorMode {
	menu,
	ship
};

struct EditorState {
	EditorMode mode = EditorMode::menu;
};