#ifndef VEC3
#define VEC3

#include <cmath>
#include <algorithm>
#include <functional>

#include "common.h"

// Vec3(point, normal, etc)
class Vec3 {
public:
	float axis[3];
	Vec3 () {}
	Vec3 (float x, float y, float z) {
		axis[AXIS_X] = x;
		axis[AXIS_Y] = y;
		axis[AXIS_Z] = z;
	}
	static inline float dot(const Vec3 &a, const Vec3 &b) {
		return (a.axis[AXIS_X] * b.axis[AXIS_X]) + (a.axis[AXIS_Y] * b.axis[AXIS_Y]) + (a.axis[AXIS_Z] * b.axis[AXIS_Z]);
	}
	static inline Vec3 add(const Vec3 &a, const Vec3 &b) {
		return Vec3(a.axis[AXIS_X] + b.axis[AXIS_X], a.axis[AXIS_Y] + b.axis[AXIS_Y], a.axis[AXIS_Z] + b.axis[AXIS_Z]);
	}
	static inline void m_add(Vec3 &a, const Vec3 &b) {
		a.axis[AXIS_X] += b.axis[AXIS_X];
		a.axis[AXIS_Y] += b.axis[AXIS_Y];
		a.axis[AXIS_Z] += b.axis[AXIS_Z];
	}
	static inline Vec3 sub(const Vec3 &a, const Vec3 &b) {
		return Vec3(a.axis[AXIS_X] - b.axis[AXIS_X], a.axis[AXIS_Y] - b.axis[AXIS_Y], a.axis[AXIS_Z] - b.axis[AXIS_Z]);
	}
	static inline Vec3 scale(const Vec3 &v, float scale) {
		return Vec3(v.axis[AXIS_X] * scale, v.axis[AXIS_Y] * scale, v.axis[AXIS_Z] * scale);
	}
	static inline void m_scale(Vec3 &v, float scale) {
		v.axis[AXIS_X] *= scale;
		v.axis[AXIS_Y] *= scale;
		v.axis[AXIS_Z] *= scale;
	}
	static inline Vec3 divide(const Vec3 &a, float divisor) {
		return Vec3(a.axis[AXIS_X] / divisor, a.axis[AXIS_Y] / divisor, a.axis[AXIS_Z] / divisor);
	}
	static inline Vec3 cross(const Vec3 &a, const Vec3 &b) {
		return Vec3((a.axis[AXIS_Y] * b.axis[AXIS_Z]) - (a.axis[AXIS_Z] * b.axis[AXIS_Y]), (a.axis[AXIS_Z] * b.axis[AXIS_X]) - (a.axis[AXIS_X] * b.axis[AXIS_Z]), (a.axis[AXIS_X] * b.axis[AXIS_Y]) - (a.axis[AXIS_Y] * b.axis[AXIS_X]));
	}
	static inline float lengthOf(const Vec3 &v) {
		return std::sqrt(std::pow(v.axis[AXIS_X], 2) + std::pow(v.axis[AXIS_Y], 2) + std::pow(v.axis[AXIS_Z], 2));
	}
	static inline Vec3 normalize(const Vec3 &v) {
		float len = Vec3::lengthOf(v);
		return Vec3(v.axis[AXIS_X] / len, v.axis[AXIS_Y] / len, v.axis[AXIS_Z] / len);
	}
	static inline void m_normalize(Vec3 &v) {
		float len = Vec3::lengthOf(v);
		v.axis[AXIS_X] /= len;
		v.axis[AXIS_Y] /= len;
		v.axis[AXIS_Z] /= len;
	}
	static inline void m_abs(Vec3 &v) {
		v.axis[AXIS_X] = std::abs(v.axis[AXIS_X]);
		v.axis[AXIS_Y] = std::abs(v.axis[AXIS_Y]);
		v.axis[AXIS_Z] = std::abs(v.axis[AXIS_Z]);
	}
	static inline void m_cap(Vec3 &v, const Vec3 &cap) {
		v.axis[AXIS_X] = std::min(v.axis[AXIS_X], cap.axis[AXIS_X]);
		v.axis[AXIS_Y] = std::min(v.axis[AXIS_Y], cap.axis[AXIS_Y]);
		v.axis[AXIS_Z] = std::min(v.axis[AXIS_Z], cap.axis[AXIS_Z]);
	}
	static inline bool eq(const Vec3 &a, const Vec3 &b, float margin = FLOAT_MARGIN_CLOSE) {
		return eqMargin(a.axis[AXIS_X], b.axis[AXIS_X], margin) && eqMargin(a.axis[AXIS_Y], b.axis[AXIS_Y], margin) && eqMargin(a.axis[AXIS_Z], b.axis[AXIS_Z], margin);
	}
};

class Vec3i {
public:
	int axis[3];
	Vec3i () {}
	Vec3i (int x, int y, int z) {
		axis[AXIS_X] = x;
		axis[AXIS_Y] = y;
		axis[AXIS_Z] = z;
	}
	Vec3i (const Vec3 &v) {
		axis[AXIS_X] = (int)v.axis[AXIS_X];
		axis[AXIS_Y] = (int)v.axis[AXIS_Y];
		axis[AXIS_Z] = (int)v.axis[AXIS_Z];
	}
	inline bool operator==(const Vec3i &b) const {
		return axis[AXIS_X] == b.axis[AXIS_X] && axis[AXIS_Y] == b.axis[AXIS_Y] && axis[AXIS_Z] == b.axis[AXIS_Z];
	}
};

namespace std {
	template<> struct hash<Vec3i> {
		inline size_t operator()(const Vec3i& x) const {
			return hash<int>()(x.axis[AXIS_X]) ^ (hash<int>()(x.axis[AXIS_Y]) << 1) ^ (hash<int>()(x.axis[AXIS_Z]) >> 1);
		}
	};
}

#endif
