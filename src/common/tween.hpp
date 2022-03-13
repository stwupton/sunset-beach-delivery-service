#pragma once

#include "types/core.hpp"

enum TweenValueType {
	float32,
	int32
};

struct Tween {
	#define MAX_TWEEN_VALUE_SIZE sizeof(Vec3<f32>)

	TweenValueType type;
	char from[MAX_TWEEN_VALUE_SIZE];
	char to[MAX_TWEEN_VALUE_SIZE];
	void *value;
	f32 duration = 0;
	f32 progress = 0;
};