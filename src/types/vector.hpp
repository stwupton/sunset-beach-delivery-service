#pragma once 

#include <cmath>

#include "types/core.hpp"

template<typename T>
struct Vec2 { 
	T x, y;
	Vec2(T x = 0, T y = 0) : x(x), y(y) {}

	Vec2<T> operator -(const Vec2<T> &other) const {
		return Vec2<T>(this->x - other.x, this->y - other.y);
	}

	Vec2<T> operator +(const Vec2<T> &other) const {
		return Vec2<T>(this->x + other.x, this->y + other.y);
	}

	f32 distanceTo(const Vec2<T> &other) const {
		return sqrt(pow(this->x - other.x, 2) + pow(this->y - other.y, 2));
	}
};

template<typename T>
struct Vec3 : Vec2<T> { 
	T z;
	Vec3(T x = 0, T y = 0, T z = 0) : Vec2<T>(x, y), z(z) {}

	Vec3<T> operator -(const Vec3<T> &other) const {
		return Vec3<T>(this->x - other.x, this->y - other.y, this->z - other.z);
	}

	Vec3<T> operator +(const Vec3<T> &other) const {
		return Vec3<T>(this->x + other.x, this->y + other.y, this->z + other.z);
	}

	f32 distanceTo(const Vec3<T> &other) const {
		return sqrt(
			pow(this->x - other.x, 2) + 
			pow(this->y - other.y, 2) + 
			pow(this->z - other.z, 2) 
		);
	}
};

struct Rgb {
	f32 r, g, b;
	Rgb(f32 r = 0, f32 g = 0, f32 b = 0) : r(r), g(g), b(b) {}
};

struct Rgba : Rgb {
	f32 a;
	Rgba(f32 r = 0, f32 g = 0, f32 b = 0, f32 a = 0) : Rgb(r, g, b), a(a) {}
};