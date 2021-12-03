#pragma once

#include "types/array.hpp"

#include "common/ship_target.hpp"
#include "common/sprite.hpp"
#include "common/weapon.hpp"

// NOTE(steven): Not  sure if we want to extend sprite because we may end up using
// data from Sprite that's not needed for the Ship
struct Ship : Sprite {
	Array<ShipTarget, 2> targets;
	Array<Weapon, 2> weapons;
};