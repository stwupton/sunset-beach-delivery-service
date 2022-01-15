#pragma once

#include "common/combat_definitions.hpp"
#include "types/core.hpp"
#include "types/vector.hpp"

struct ShipTarget {
	HealthValue maxHealth = 0;
	HealthValue health = 0;
	Vec3<f32> position;
	f32 selectRadius;
};