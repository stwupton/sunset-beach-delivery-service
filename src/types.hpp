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