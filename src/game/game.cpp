#pragma once

#include <cmath>

#include "common/asset_definitions.hpp"
#include "common/game_state.hpp"
#include "common/sprite.hpp"
#include "common/window_config.hpp"
#include "common/ui_element.hpp"
#include "game/utils.cpp"
#include "types/core.hpp"
#include "types/nullable.cpp"
#include "utils/reducer.hpp"

class Game {
public:
	void load(GameState *gameState) {
		gameState->loadQueue.push(TextureAssetId::ship);
		gameState->loadQueue.push(TextureAssetId::enemyShip);
		gameState->loadQueue.push(TextureAssetId::background);
	}

	void setup(GameState *gameState) {
		// Ally ship
		{
			Ship ship = {};
			ship.assetId = TextureAssetId::ship;
			ship.position = Vec3<f32>(0.0f, -300.0f, 0.4f);
			ship.scale = Vec2<f32>(0.5f, 0.5f);
			ship.angle = -95.0f;
			
			ship.targets.push(
				{ CombatParty::ally, 100, 100, Vec3<f32>(-300.0f, -200.0f), 50.0f }
			);

			ship.weapons.push(
				{ CombatParty::ally, Vec3<f32>(60.0f, -300.0f), 50.0f, 20, 180.0f }
			);

			gameState->ships.push(ship);
		}

		// Enemy ship
		{
			Ship enemyShip = {};
			enemyShip.assetId = TextureAssetId::enemyShip;
			enemyShip.position = Vec3<f32>(0.0f, 300.0f, 0.4f);
			enemyShip.scale = Vec2<f32>(0.2f, 0.2f);
			enemyShip.angle = -180.0f;

			enemyShip.targets.push(
				{ CombatParty::enemy, 200, 100, Vec3<f32>(0.0f, 300.0f), 50.0f }
			);

			enemyShip.weapons.push(
				{ CombatParty::enemy, Vec3<f32>(-60.0f, -300.0f), 50.0f }
			);

			gameState->ships.push(enemyShip);
		}
	}

	void update(GameState *gameState, f32 delta) {
		SpriteBuffer &sprites = gameState->sprites;
		sprites.clear();

		// TODO(steven): For some reason, if we add the background after to the sprite buffer
		// after the ships, the ships cut a rectangle in the background. I suspect we
		// are doing something wrong in DirectXRenderer.createBlendState().
		Sprite background = {};
		background.assetId = TextureAssetId::background;
		background.position = Vec3<f32>(0.0f, 0.0f, 0.9f);
		background.scale = Vec2<f32>(1.3f, 1.3f);
		sprites.push(background);

		for (const Ship &ship : gameState->ships) {
			sprites.push(ship);
		}

		gameState->uiElements.clear();
		this->handleUserTargeting(gameState);
		this->updateWeaponCooldowns(gameState, delta);
		this->updateProjectiles(gameState, delta);
		this->processEvents(gameState);
		this->updateTargets(gameState);
		this->updateAimlessProjectiles(gameState, delta);
		this->renderCombatVisuals(gameState);
		this->debugUI(gameState, delta);
	}

protected:
	// TODO(steven): Cleanup
	void handleUserTargeting(GameState *gameState) const {
		for (Ship &ship : gameState->ships) {
			Weapon *targetingWeapon = nullptr;
			Vec2<f32> weaponScreenPosition;

			for (Weapon &weapon : ship.weapons) {
				if (weapon.party == CombatParty::ally) {
					if (gameState->input.primaryButton.down) {
						weaponScreenPosition = gameToScreen(weapon.position);
						const f32 difference = gameState->input.primaryButton.start.distanceTo(weaponScreenPosition);

						if (difference < weapon.selectRadius) {
							targetingWeapon = &weapon;
							weapon.firing = false;
							break;
						}
					} else if (weapon.target != nullptr) {
						weapon.firing = true;
					}
				}
			}

			if (targetingWeapon != nullptr) {
				ShipTarget *enemyTarget = nullptr;
				Vec2<f32> targetScreenPosition;

				for (ShipTarget &target : ship.targets) {
					if (target.party == CombatParty::enemy && gameState->input.primaryButton.down) {
						targetScreenPosition = gameToScreen(target.position);
						const f32 difference = gameState->input.mouse.distanceTo(targetScreenPosition);
						if (difference < target.selectRadius) {
							enemyTarget = &target;
							break;
						}
					}
				}

				targetingWeapon->target = enemyTarget;

				// Draw targeting line
				UILineData drawLine = {};
				drawLine.start = weaponScreenPosition;

				if (targetingWeapon->target != nullptr) {
					drawLine.end = targetScreenPosition;
				} else {
					drawLine.end = gameState->input.mouse;
					targetingWeapon->firing = false;
				}

				drawLine.thickness = 10.0f;
				drawLine.color = Rgba(1.0f, 0.0f, 0.0f, 1.0f);			
				gameState->uiElements.push(drawLine);
			}
		}
	}

	void renderCombatVisuals(GameState *gameState) const {
		UIElementBuffer &uiElements = gameState->uiElements;

		// Draw projectiles
		for (const Projectile &projectile : gameState->projectiles) {
			UICircleData bullet = {};
			bullet.position = gameToScreen(projectile.position);
			bullet.radius = 1.0f;
			bullet.thickness = 10.0f;
			bullet.color = Rgba(0.0f, 0.0f, 1.0f, 1.0f);
			uiElements.push(bullet);
		}

		// Draw aimless prjectiles
		for (const AimlessProjectile &aimless : gameState->aimlessProjectiles) {
			UICircleData bullet = {};
			bullet.position = gameToScreen(aimless.position);
			bullet.radius = 1.0f;
			bullet.thickness = 10.0f;
			bullet.color = Rgba(0.0f, 0.0f, 1.0f, 1.0f);
			uiElements.push(bullet);
		}

		for (Ship &ship : gameState->ships) {
			// Draw weapons
			for (const Weapon &weapon : ship.weapons) {
				UICircleData weaponIndicator = {};
				weaponIndicator.position = gameToScreen(weapon.position);
				weaponIndicator.thickness = 10.0f;
				weaponIndicator.radius = weapon.selectRadius;

				if (weapon.party == CombatParty::ally) {
					weaponIndicator.color = Rgba(0.0f, 1.0f, 0.0f, 1.0f);
				} else {
					weaponIndicator.color = Rgba(1.0f, 0.0f, 0.0f, 1.0f);
				}

				uiElements.push(weaponIndicator);

				if (weapon.firing) {
					UITextData firingText = {};
					firingText.color = Rgba(1.0f, 0.0f, 0.0f, 1.0f);
					firingText.fontSize = 10.0f;
					firingText.height = 10.0f;
					firingText.width = 100.0f;
					firingText.position = weaponIndicator.position + Vec2<f32>(-15.0f, -10.0f);
					firingText.text = L"FIRING";
					uiElements.push(firingText);
				}
			}

			// Draw combat ship targets
			// TODO(steven): Just using debug circles for now, represent them another way
			for (const ShipTarget &target : ship.targets) {
				UICircleData targetPoint = {};
				targetPoint.position = gameToScreen(target.position);
				targetPoint.thickness = 10.0f;
				targetPoint.radius = target.selectRadius;
				targetPoint.style = UICircleStyle::dotted;

				if (target.party == CombatParty::ally) {
					targetPoint.color = Rgba(0.0f, 1.0f, 0.0f, 1.0f);
				} else {
					targetPoint.color = Rgba(1.0f, 0.0f, 0.0f, 1.0f);
				}

				uiElements.push(targetPoint);

				UILineData healthBar = {};
				const f32 healthScale = (f32)target.health / target.maxHealth;
				healthBar.start = targetPoint.position + Vec2<f32>(-50.0f, -100.0f);
				healthBar.end = healthBar.start + Vec2<f32>(100.0f * healthScale, 0.0f);
				healthBar.color = Rgba(abs(healthScale - 1.0f), healthScale, 0.0f, 1.0f);
				healthBar.thickness = 20.0f;
				uiElements.push(healthBar);
			}
		}
	}

	void debugUI(GameState *gameState, f32 delta) const {
		UIElementBuffer &uiElements = gameState->uiElements;

		wchar_t textBuffer[100] = {};

		u8 fps = round(1 / delta);
		swprintf_s(textBuffer, L"FPS: %d", fps);
		UITextData fpsCount = {};
		fpsCount.text = textBuffer;
		fpsCount.fontSize = 30.0f;
		fpsCount.position = 0;
		fpsCount.width = 300.0f;
		fpsCount.height = 40.0f;
		fpsCount.color = Rgba(1.0f, 0.0f, 0.0f, 1.0f);
		uiElements.push(fpsCount);

		swprintf_s(textBuffer, L"Mouse Down: %d", gameState->input.primaryButton.down);
		UITextData buttonMessage = {};
		buttonMessage.text = textBuffer;
		buttonMessage.fontSize = 30.0f;
		buttonMessage.position = Vec2<f32>(0.0f, 40.0f);
		buttonMessage.width = 300.0f;
		buttonMessage.height = 40.0f;
		buttonMessage.color = Rgba(1.0f, 0.0f, 0.0f, 1.0f);
		uiElements.push(buttonMessage);

		const Vec2<f32> mouse = gameState->input.mouse;
		swprintf_s(textBuffer, L"X: %d, Y: %d", (u32)mouse.x, (u32)mouse.y);
		UITextData mouseCoords = {};
		mouseCoords.text = textBuffer;
		mouseCoords.fontSize = 30.0f;
		mouseCoords.position = Vec2<f32>(0.0f, 80.0f);
		mouseCoords.width = 300.0f;
		mouseCoords.height = 40.0f;
		mouseCoords.color = Rgba(1.0f, 0.0f, 0.0f, 1.0f);
		uiElements.push(mouseCoords);
	}

	void processEvents(GameState *gameState) const {
		for (const TargetDestroyedEvent &event : gameState->events.targetDestroyed) {
			for (Ship &ship : gameState->ships) {
				for (Weapon &weapon : ship.weapons) {
					if (weapon.target == event.target) {
						weapon.firing = false;
						weapon.target = nullptr;
						weapon.cooldownTick = weapon.cooldown;
					}
				}
			}

			Reducer reducer(&gameState->projectiles);
			for (Projectile &projectile : gameState->projectiles) {
				reducer.next(&projectile);

				if (projectile.target == event.target) {
					reducer.remove();

					AimlessProjectile aimless = {};
					aimless.position = projectile.position;
					aimless.speed = projectile.speed;

					const Vec3 direction = (projectile.target->position - projectile.position).normalized();
					aimless.direction = direction;

					gameState->aimlessProjectiles.push(aimless);
				}
			}

			reducer.finish();
		}
	}

	void updateAimlessProjectiles(GameState *gameState, f32 delta) const {
		Reducer reducer(&gameState->aimlessProjectiles);

		for (AimlessProjectile &aimless : gameState->aimlessProjectiles) {
			reducer.next(&aimless);

			aimless.tick += delta * 1000;
			if (aimless.tick >= aimless.lifetime) {
				reducer.remove();
			} else {
				aimless.position += aimless.direction * aimless.speed * delta;
			}
		}

		reducer.finish();
	}

	void updateProjectiles(GameState *gameState, f32 delta) const {
		Reducer reducer(&gameState->projectiles);

		for (Projectile &projectile : gameState->projectiles) {
			reducer.next(&projectile);

			const Vec3 diff = projectile.target->position - projectile.position;
			const Vec3 direction = diff.normalized();
			const f32 distance = diff.magnitude();

			if (distance < 10.0f) {
				reducer.remove();

				HealthValue healthAfterDamage = projectile.target->health - projectile.damage;

				projectile.target->health = max(0, healthAfterDamage);
				if (projectile.target->health == 0) {
					TargetDestroyedEvent event = {};
					event.target = projectile.target;
					gameState->events.targetDestroyed.push(event);
				}
			} else {
				projectile.position += direction * projectile.speed * delta;
			}
		}

		reducer.finish();
	}

	void updateTargets(GameState *gameState) const {
		for (Ship &ship : gameState->ships) {
			Reducer reducer(&ship.targets);

			for (ShipTarget &target : ship.targets) {
				reducer.next(&target);
				if (target.health == 0) {
					reducer.remove();
				}
			}

			reducer.finish();
		}
	}

	void updateWeaponCooldowns(GameState *gameState, f32 delta) const {
		for (Ship &ship : gameState->ships) {
			for (Weapon &weapon : ship.weapons) {
				if (!weapon.firing) {
					continue;
				}

				weapon.cooldownTick += delta * 1000;
				if (weapon.cooldownTick >= weapon.cooldown) {
					weapon.cooldownTick = fmod(weapon.cooldownTick, weapon.cooldown);

					Projectile projectile = {};
					projectile.damage = weapon.damage;
					projectile.speed = weapon.projectileSpeed;
					projectile.target = weapon.target;
					projectile.position = weapon.position;

					gameState->projectiles.push(projectile);
				}
			}
		}
	}
};