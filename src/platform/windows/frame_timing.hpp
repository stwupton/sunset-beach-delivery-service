#pragma once

#include "types/core.hpp"

struct FrameTiming {
	f32 delta = 0.0f;
	s64 frequency = 1;
	s64 previousTime = 0;
};