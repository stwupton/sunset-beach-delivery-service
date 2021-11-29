#pragma once

#include "common/combat_definitions.hpp"
#include "common/combat_party.hpp"
#include "types/core.hpp"
#include "types/vector.hpp"

struct ShipTarget {
	CombatParty party = CombatParty::none;
	HealthValue maxHealth = 0;
	HealthValue health = 0;
	Vec3<f32> position;
	f32 selectRadius;
};