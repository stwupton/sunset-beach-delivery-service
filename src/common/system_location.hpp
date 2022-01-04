#pragma once

#include "types/core.hpp"
#include "types/string.hpp"
#include "types/vector.hpp"

struct SystemLocation {
	String16<32> name;
	Rgba color = Rgba(1.0f, 1.0f, 1.0f, 1.0f);
	f32 orbitingDistance = 200.0f;
	f32 orbitAngle = 0.0f;
	const SystemLocation *orbiting = nullptr;
	f32 orbitSpeed = 1.0f;
	Vec2<f32> position;
	f32 radius = 1.0f;
};