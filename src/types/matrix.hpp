#pragma once

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

	Mat4x4<T> scale(T x = 1, T y = 1, T z = 1, T w = 1) const {
		Mat4x4<f32> result = *this;
		result.x0 = x;
		result.y1 = y;
		result.z2 = z;
		result.w3 = w;
		return result;
	}

	Mat4x4<T> translate(T x = 0, T y = 0, T z = 0) const {
		Mat4x4<T> result = *this;
		result.x3 = result.x0 * x + result.x1 * y + result.x2 * z + result.x3; 
		result.y3 = result.y0 * x + result.y1 * y + result.y2 * z + result.y3; 
		result.z3 = result.z0 * x + result.z1 * y + result.z2 * z + result.z3; 
		return result;
	}

	Mat4x4<T> rotate(T angle) const {
		const T c = cos(angle);
		const T s = sin(angle);

		Mat4x4<T> rotate;
		rotate.x0 = c;
		rotate.y0 = s;

		rotate.x1 = -s;
		rotate.y1 = c;

		Mat4x4<T> result = *this;
		result.x0 = this->x0 * rotate.x0 + this->x1 * rotate.y0;
		result.y0 = this->y0 * rotate.x0 + this->y1 * rotate.y0;
		result.z0 = this->z0 * rotate.x0 + this->z1 * rotate.y0;

		result.x1 = this->x0 * rotate.x1 + this->x1 * rotate.y1;
		result.y1 = this->y0 * rotate.x1 + this->y1 * rotate.y1;
		result.z1 = this->z0 * rotate.x1 + this->z1 * rotate.y1;

		return result;
	}
};