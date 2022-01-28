#pragma once

#include "types/core.hpp"
#include "common/system_location.hpp"

struct Shipment {
	u32 cashAward = 0;
	SystemLocation *from = nullptr;
	SystemLocation *to = nullptr;
	f32 weight;
};