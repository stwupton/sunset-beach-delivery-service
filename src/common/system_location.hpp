#pragma once

#include "types/core.hpp"
#include "types/string.hpp"
#include "types/vector.hpp"

struct SystemLocation {
	String16<32> name;
	Rgba color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
	struct {
		f32 angle = 0.0f;
		f32 speed = 1.0f;
		f32 distance = 10.0f;
	} orbit;
	Vec2<f32> position;
	f32 radius = 1.0f;
	bool isMoon;
};