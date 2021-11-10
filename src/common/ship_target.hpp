#pragma once

#include "types/core.hpp"
#include "types/vector.hpp"

enum CombatParty : u8 {
	ally,
	enemy,
	none
};

struct ShipTarget {
	CombatParty party = CombatParty::none;
	u16 maxHealth = 0;
	u16 health = 0;
	Vec2<f32> position;
};