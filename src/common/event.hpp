#pragma once

#include "common/ship_target.hpp"
#include "types/array.hpp"
#include "types/core.hpp"

struct TargetDestroyedEvent {
	ShipTarget *target;
};

struct Events {
	Array<TargetDestroyedEvent, 10> targetDestroyed;	
};