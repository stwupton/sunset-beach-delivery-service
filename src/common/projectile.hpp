#pragma once 

#include "common/ship_target.hpp"
#include "types/core.hpp"

struct Projectile {
	Vec3<f32> position;
	// TODO(steven): target may not always be valid. For example: if it gets destroyed
	ShipTarget *target;
	u16 damage;
	f32 speed = 1.0f;
	bool destroyed = false;
};