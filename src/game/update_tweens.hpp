#pragma once

#include "common/game_state.hpp"
#include "types/core.hpp"
#include "utils/reducer.hpp"


template<typename T>
bool updateTween(Tween *tween, f32 delta) {
	#define PTR_CAST(type, ptr) *(type*)ptr

	const T difference = PTR_CAST(T, tween->to) - PTR_CAST(T, tween->from);
	tween->progress += delta / tween->duration;
	tween->progress = min(1.0f, max(0.0, tween->progress));
	PTR_CAST(T, tween->value) = PTR_CAST(T, tween->from) + difference * tween->progress;
	return tween->progress == 1.0f;
}

void updateTweens(GameState *gameState, f32 delta) {
	Reducer reducer(&gameState->tweens);

	for (Tween &tween : gameState->tweens) {
		reducer.next(&tween);

		bool complete = false;
		switch (tween.type) {
			case TweenValueType::float32: {
				complete = updateTween<f32>(&tween, delta);
			} break;

			case TweenValueType::int32: {
				complete = updateTween<s32>(&tween, delta);
			} break;
		}

		if (complete) {
			reducer.remove();
		}
	}

	reducer.finish();
}