#pragma once

#include "types/string.hpp"

struct SaveData {
	bool pending = false;
	string16<64> path;
	char buffer[4096];
	size_t size;
};