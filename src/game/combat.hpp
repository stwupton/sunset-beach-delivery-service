#pragma once

#include <cmath>

#include "common/game_state.hpp"
#include "game/utils.hpp"
#include "types/core.hpp"
#include "utils/reducer.hpp"

namespace Combat {
	// Forward declerations
	void updateWeaponCooldowns(GameState *gameState, f32 delta);
	void updateProjectiles(GameState *gameState, f32 delta);
	void processEvents(GameState *gameState);
	void updateTargets(GameState *gameState);
	template<size_t Size> void updateShips(Array<Ship, Size> *ships);
	void updateAimlessProjectiles(GameState *gameState, f32 delta);
	void renderCombatVisuals(GameState *gameState);
	void handleUserTargeting(GameState *gameState);
	void drawWeapon(GameState *gameState, const Weapon &weapon, const Rgba &color);
	void drawTarget(GameState *gameState, const ShipTarget &target, const Rgba &color);
	bool isShipAlive(Ship &ship);

	template<typename T, size_t Size>
	void addSprites(SpriteBuffer *sprites, Array<T, Size> toRender) {
		for (const T &sprite : toRender) {
			sprites->push((Sprite)sprite);
		}
	}

	void setup(GameState *gameState) {
		gameState->textureLoadQueue.push(TextureAssetId::ship);
		gameState->textureLoadQueue.push(TextureAssetId::enemyShip);
		gameState->textureLoadQueue.push(TextureAssetId::background);

		// Ally ship
		{
			Ship ship = {};
			ship.assetId = TextureAssetId::ship;
			ship.position = Vec3(0.0f, -300.0f, 0.4f);
			ship.scale = Vec2(0.5f, 0.5f);
			ship.angle = -95.0f;
			
			ShipTarget target = {};
			target.health = 100;
			target.maxHealth = 100;
			target.position = Vec3(0.0f, -400.0f);
			target.selectRadius = 50.0f;
			ship.targets.push(target);

			Weapon weapon = {};
			weapon.position = Vec3(200.0f, -300.0f);
			weapon.selectRadius = 50.0f;
			weapon.damage = 20;
			weapon.projectileSpeed = 180.0f;
			ship.weapons.push(weapon);

			gameState->allyShips.push(ship);
		}

		// Enemy ship
		{
			Ship enemyShip = {};
			enemyShip.assetId = TextureAssetId::enemyShip;
			enemyShip.position = Vec3(0.0f, 300.0f, 0.4f);
			enemyShip.scale = Vec2(0.2f, 0.2f);
			enemyShip.angle = -180.0f;

			ShipTarget target = {};
			target.maxHealth = 200;
			target.health = 100;
			target.position = Vec3(0.0f, 400.0f);
			target.selectRadius = 50.0f;
			enemyShip.targets.push(target);

			Weapon weapon = {};
			weapon.position = Vec3(0.0f, 300.0f);
			weapon.selectRadius = 50.0f;
			enemyShip.weapons.push(weapon);

			gameState->enemyShips.push(enemyShip);
		}
	}

	void update(GameState *gameState, f32 delta) {
		SpriteBuffer &sprites = gameState->sprites;

		// NOTE(steven): Background needs to be added before any sprites that have
		// transparent textures are added.
		Sprite background = {};
		background.assetId = TextureAssetId::background;
		background.position = Vec3<f32>(0.0f, 0.0f, 0.9f);
		background.scale = Vec2<f32>(1.3f, 1.3f);
		sprites.push(background);

		addSprites(&sprites, gameState->allyShips);
		addSprites(&sprites, gameState->enemyShips);

		updateWeaponCooldowns(gameState, delta);
		updateProjectiles(gameState, delta);
		processEvents(gameState);
		updateTargets(gameState);
		updateShips(&gameState->allyShips);
		updateShips(&gameState->enemyShips);
		updateAimlessProjectiles(gameState, delta);
		renderCombatVisuals(gameState);
		handleUserTargeting(gameState);
	}

	// TODO(steven): Cleanup
	void handleUserTargeting(GameState *gameState) {
		for (Ship &ship : gameState->allyShips) {
			Weapon *targetingWeapon = nullptr;
			Vec2<f32> weaponScreenPosition;
			Vec2<f32> targetScreenPosition;

			for (Weapon &weapon : ship.weapons) {
				if (gameState->input.primaryButton.down) {
					weaponScreenPosition = gameToScreen(weapon.position);
					f32 difference = gameState->input.primaryButton.start.distanceTo(weaponScreenPosition);

					if (difference < weapon.selectRadius) {
						targetingWeapon = &weapon;
						weapon.firing = false;

						for (Ship &enemyShip : gameState->enemyShips) {
							for (ShipTarget &target : enemyShip.targets) {
								targetScreenPosition = gameToScreen(target.position);
								difference = gameState->input.mouse.distanceTo(targetScreenPosition) ;

								if (difference < target.selectRadius) {
									weapon.target = &target;
									break;
								} else {
									weapon.target = nullptr;
								}
							}
						}

						break;
					}
				} else if (weapon.target != nullptr) {
					weapon.firing = true;
				}
			}

			if (targetingWeapon != nullptr) {
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

	void renderCombatVisuals(GameState *gameState) {
		UIElementBuffer &uiElements = gameState->uiElements;

		// Draw projectiles
		for (const Projectile &projectile : gameState->projectiles) {
			UICircleData bullet = {};
			bullet.position = gameToScreen(projectile.position);
			bullet.radius = 1.0f;
			bullet.thickness = 10.0f;
			bullet.strokeColor = Rgba(0.0f, 0.0f, 1.0f, 1.0f);
			uiElements.push(bullet);
		}

		// Draw aimless prjectiles
		for (const AimlessProjectile &aimless : gameState->aimlessProjectiles) {
			UICircleData bullet = {};
			bullet.position = gameToScreen(aimless.position);
			bullet.radius = 1.0f;
			bullet.thickness = 10.0f;
			bullet.strokeColor = Rgba(0.0f, 0.0f, 1.0f, 1.0f);
			uiElements.push(bullet);
		}

		// Draw ally ships
		for (Ship &ship : gameState->allyShips) {
			const Rgba allyColor(0.0f, 1.0f, 0.0f, 1.0f);

			for (const Weapon &weapon : ship.weapons) {
				drawWeapon(gameState, weapon, allyColor);
			}

			for (const ShipTarget &target : ship.targets) {
				drawTarget(gameState, target, allyColor);
			}
		}

		// Draw enemy ships
		for (Ship &ship : gameState->enemyShips) {
			const Rgba enemyColor(1.0f, 0.0f, 0.0f, 1.0f);

			for (const Weapon &weapon : ship.weapons) {
				drawWeapon(gameState, weapon, enemyColor);
			}

			for (const ShipTarget &target : ship.targets) {
				drawTarget(gameState, target, enemyColor);
			}
		}
	}

	// Draw combat ship targets
	// TODO(steven): Just using debug circles for now, represent them another way
	void drawTarget(GameState *gameState, const ShipTarget &target, const Rgba &color) {
		UIElementBuffer &uiElements = gameState->uiElements;
		UICircleData targetPoint = {};
		targetPoint.position = gameToScreen(target.position);
		targetPoint.thickness = 10.0f;
		targetPoint.radius = target.selectRadius;
		targetPoint.strokeStyle = UICircleStrokeStyle::dotted;
		targetPoint.strokeColor = color;

		uiElements.push(targetPoint);

		UILineData healthBar = {};
		const f32 healthScale = (f32)target.health / target.maxHealth;
		healthBar.start = targetPoint.position + Vec2<f32>(-50.0f, -100.0f);
		healthBar.end = healthBar.start + Vec2<f32>(100.0f * healthScale, 0.0f);
		healthBar.color = Rgba(abs(healthScale - 1.0f), healthScale, 0.0f, 1.0f);
		healthBar.thickness = 20.0f;
		uiElements.push(healthBar);
	}

	void drawWeapon(GameState *gameState, const Weapon &weapon, const Rgba &color) {
		UIElementBuffer &uiElements = gameState->uiElements;

		UICircleData weaponIndicator = {};
		weaponIndicator.position = gameToScreen(weapon.position);
		weaponIndicator.thickness = 10.0f;
		weaponIndicator.radius = weapon.selectRadius;
		weaponIndicator.strokeColor = color;
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

	void processEvents(GameState *gameState) {
		for (const TargetDestroyedEvent &event : gameState->events.targetDestroyed) {
			for (Ship &ship : gameState->allyShips) {
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

	void updateAimlessProjectiles(GameState *gameState, f32 delta) {
		Reducer reducer(&gameState->aimlessProjectiles);

		for (AimlessProjectile &aimless : gameState->aimlessProjectiles) {
			reducer.next(&aimless);

			aimless.tick += delta;
			if (aimless.tick >= aimless.lifetime) {
				reducer.remove();
			} else {
				aimless.position += aimless.direction * aimless.speed * delta;
			}
		}

		reducer.finish();
	}

	void updateProjectiles(GameState *gameState, f32 delta) {
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

					gameState->soundLoadQueue.push(SoundAssetId::doomDeath);
				}
			} else {
				projectile.position += direction * projectile.speed * delta;
			}
		}

		reducer.finish();
	}

	template<size_t Size>
	void updateShips(Array<Ship, Size> *ships) {
		Reducer reducer(ships);

		for (Ship &ship : *ships) {
			reducer.next(&ship);
			if (!isShipAlive(ship)) {
				reducer.remove();
			}
		}

		reducer.finish();
	}

	bool isShipAlive(Ship &ship) {
		for (const ShipTarget &target : ship.targets) {
			if (target.health > 0) {
				return true;
			}
		}
		return false;
	}

	void updateTargets(GameState *gameState) {
		for (Ship &ship : gameState->enemyShips) {
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

	void updateWeaponCooldowns(GameState *gameState, f32 delta) {
		for (Ship &ship : gameState->allyShips) {
			for (Weapon &weapon : ship.weapons) {
				if (!weapon.firing) {
					continue;
				}

				weapon.cooldownTick += delta;
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