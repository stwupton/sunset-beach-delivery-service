#pragma once

#include "common/ship.hpp"

template<typename T>
struct TemplateData {
	String16<32> displayName;
	String16<32> fileName;
	T data;
};

typedef Array<TemplateData<Ship>, 2> ShipTemplates;

struct Templates {
	ShipTemplates ships;
};