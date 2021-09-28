#pragma once

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef float f32;
typedef double f64;

template<typename T>
struct Mat4x4 {
	T x0; T y0; T z0; T w0;
	T x1; T y1; T z1; T w1;
	T x2; T y2; T z2; T w2;
	T x3; T y3; T z3; T w3;

	Mat4x4(
		T x0 = 1, T x1 = 0, T x2 = 0, T x3 = 0,
		T y0 = 0, T y1 = 1, T y2 = 0, T y3 = 0,
		T z0 = 0, T z1 = 0, T z2 = 1, T z3 = 0,
		T w0 = 0, T w1 = 0, T w2 = 0, T w3 = 1
	) {
		this->x0 = x0; this->x1 = x1; this->x2 = x2; this->x3 = x3;
		this->y0 = y0; this->y1 = y1; this->y2 = y2; this->y3 = y3;
		this->z0 = z0; this->z1 = z1; this->z2 = z2; this->z3 = z3;
		this->w0 = w0; this->w1 = w1; this->w2 = w2; this->w3 = w3;
	}

	Mat4x4<T> scale(T x = 1, T y = 1, T z = 1, T w = 1) {
		Mat4x4<f32> result = *this;
		result.x0 = x;
		result.y1 = y;
		result.z2 = z;
		result.w3 = w;
		return result;
	}

	Mat4x4<T> translate(T x = 0, T y = 0, T z = 0) {
		Mat4x4<T> result = *this;
		result.x3 = result.x0 * x + result.x1 * y + result.x2 * z + result.x3; 
		result.y3 = result.y0 * x + result.y1 * y + result.y2 * z + result.y3; 
		result.z3 = result.z0 * x + result.z1 * y + result.z2 * z + result.z3; 
		return result;
	}
};

template<typename T>
struct Vec2 { 
	T x, y;
	Vec2(T x = 0, T y = 0) : x(x), y(y) {}
};

template<typename T>
struct Vec3 : Vec2<T> { 
	T z;
	Vec3(T x = 0, T y = 0, T z = 0) : Vec2<T>(x, y), z(z) {}
};

template<typename T>
struct Vec4 : Vec3<T> {
	T w;
	Vec4(T x = 0, T y = 0, T z = 0, T w = 0) : Vec3<T>(x, y, z), w(w) {}
};

struct Rgb {
	f32 r, g, b;
	Rgb(f32 r = 0, f32 g = 0, f32 b = 0) : r(r), g(g), b(b) {}
};

struct Rgba : Rgb {
	f32 a;
	Rgba(f32 r = 0, f32 g = 0, f32 b = 0, f32 a = 0) : Rgb(r, g, b), a(a) {}
};