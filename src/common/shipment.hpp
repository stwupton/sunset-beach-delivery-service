#pragma once

#include "types/core.hpp"
#include "common/system_location.hpp"

typedef u16 CreditValue;

struct Shipment {
	CreditValue creditAward = 0;
	SystemLocation *from = nullptr;
	SystemLocation *to = nullptr;
	f32 weight;
};