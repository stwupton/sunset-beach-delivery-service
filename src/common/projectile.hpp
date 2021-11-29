#pragma once 

#include "common/combat_definitions.hpp"
#include "common/ship_target.hpp"
#include "types/core.hpp"

struct Projectile {
	Vec3<f32> position;
	// TODO(steven): target may not always be valid. For example: if it gets destroyed
	ShipTarget *target;
	HealthValue damage;
	f32 speed = 1.0f;
};

struct AimlessProjectile {
	Vec3<f32> position;
	Vec3<f32> direction;
	f32 speed = 1.0f;
	f32 lifetime = 10000.0f;
	f32 tick = 0.0f;
};