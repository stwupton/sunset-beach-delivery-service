#pragma once

#include "common/combat_party.hpp"
#include "types/core.hpp"
#include "types/vector.hpp"

struct Weapon {
	CombatParty party;
	Vec2<f32> position;
};