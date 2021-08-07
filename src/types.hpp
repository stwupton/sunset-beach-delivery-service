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
struct Vec3 { 
	union { T x, r; };
	union { T y, g; };
	union { T z, b; };
  Vec3(T x = 0, T y = 0, T z = 0) : x(x), y(y), z(z) {}
};

template<typename T>
struct Vec4 : Vec3<T> {
  union { T w, a; };
  Vec4(T x = 0, T y = 0, T z = 0, T w = 0) : Vec3<T>(x, y, z), w(w) {}
};

typedef Vec4<f32> Color;