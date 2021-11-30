#pragma once

#include "common/combat_party.hpp"
#include "types/core.hpp"
#include "types/vector.hpp"

struct ShipTarget {
	CombatParty party = CombatParty::none;
	u16 maxHealth = 0;
	u16 health = 0;
	Vec3<f32> position;
	f32 selectRadius;
};