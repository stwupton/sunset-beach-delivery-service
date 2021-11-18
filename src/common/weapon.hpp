#pragma once

#include "common/combat_party.hpp"
#include "common/ship_target.hpp"
#include "types/core.hpp"
#include "types/nullable.cpp"
#include "types/vector.hpp"

struct Weapon {
	CombatParty party;
	Vec3<f32> position;
	f32 selectRadius;
	ShipTarget *target;
	bool firing = false;
};