#pragma once

#include <cstdio>
#include <Windows.h>

#include "common/asset_definitions.hpp"
#include "types/array.hpp"
#include "types/core.hpp"
#include "types/vector.hpp"

#define MAX_ENTITY_COUNT 256

struct Transform {
	Vec3<f32> position;
	Vec2<f32> scale = Vec2(1.0f, 1.0f);
	f32 angle;
};

struct Sprite {
	TextureAssetId id;
};

struct Velocity {
	Vec2<f32> direction;
};

typedef u8 Component_Id;

static Component_Id component_count = 0;

template<typename T>
Component_Id get_component_id() {
	static Component_Id id = component_count++;
	return id;
}

typedef u16 Entity_Id;
typedef u16 Entity_Version;

struct Entity_Uid {
	Entity_Id id;
	Entity_Version version;
};

template<typename T, size_t Size>
struct Component_Pool {
	size_t sparse_indices[Size] = {};
	Array<T, Size> components = {};

	T *add(Entity_Id entity_id, const T &component) {
		this->sparse_indices[(size_t)entity_id] = this->components.length;
		return &this->components.push(component);
	}
};

struct Entity {
	Entity_Uid uid;
	u32 components = 0;

	template<typename ...Ts>
	bool has() const {
		Component_Id ids[] = { get_component_id<Ts>()... };

		u32 match = 0;
		for (size_t i = 0; i < sizeof...(Ts); i++) {
			match |= (0x1 << ids[i]);
		}

		return (this->components & match) == match;
	}
};

template<size_t Size>
struct Entity_State {
	using Any_Component_Pool = Component_Pool<void*, Size>;

	Array<Entity, Size> entities = {};
	Any_Component_Pool *component_pools[8] = {};

	~Entity_State() {
		for (Any_Component_Pool *pool : this->component_pools) {
			delete pool;
		}
	}

	Entity_Id create_entity() {
		const Entity_Id id = this->entities.length++;
		this->entities[id].uid.id = id;
		return id;
	}

	template<typename T>
	T *add_component(Entity_Id entity_id, const T &component = {}) {
		const Component_Id component_id = get_component_id<T>();

		Entity *entity = &this->entities[(size_t)entity_id];
		entity->components |= 0x1 << component_id;

		Component_Pool<T, Size> *pool = (Component_Pool<T, Size>*)this->component_pools[(size_t)component_id];
		return pool->add(entity_id, component);
	}

	template<typename T>
	void add_component_pool() {
		Component_Id id = get_component_id<T>();
		component_pools[(size_t)id] = (Any_Component_Pool*)(new Component_Pool<T, Size>()); 
	}

	template<typename T>
	T &get_component(Entity_Id entity_id) {
		const Component_Id component_id = get_component_id<T>();
		Component_Pool<T, Size> *pool = (Component_Pool<T, Size>*)this->component_pools[(size_t)component_id];
		const size_t component_index = pool->sparse_indices[(size_t)entity_id];
		return pool->components[component_index];
	}
};