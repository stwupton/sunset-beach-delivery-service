#pragma once

#include "common/combat_definitions.hpp"
#include "common/ship_target.hpp"
#include "types/core.hpp"
#include "types/nullable.hpp"
#include "types/vector.hpp"

struct Weapon {
	Vec3<f32> position;
	f32 selectRadius;
	HealthValue damage;
	f32 projectileSpeed;
	f32 cooldown = 1.0f;
	f32 cooldownTick = 0.0f;
	ShipTarget *target;
	bool firing = false;
};