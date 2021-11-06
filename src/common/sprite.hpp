#pragma once

#include "common/asset_definitions.hpp"
#include "types/core.hpp"
#include "types/vector.hpp"

struct Sprite {
	Vec3<f32> position;
	Vec2<f32> scale;
	f32 angle;
	TextureAssetId assetId;
};