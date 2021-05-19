#ifndef LIGHT
#define LIGHT

#include <cmath>
#include <omp.h>
#include "vec3.h"

#define LIGHT_CAST_RAND_ADD 16
#define LIGHT_CAST_RADIUS_MARGIN 0.0005
class Light {
public:
	Vec3 pos;
	Vec3 color;
	float lum;
	bool shadowCast;
	unsigned int radius;

	Light () {}
	Light (Vec3 color, float lum, Vec3 pos, bool shadowCast) : pos(pos), lum(lum), shadowCast(shadowCast) {
		this->color = color;
		Vec3::normalize(this->color);
		radius = distance(LIGHT_CAST_RADIUS_MARGIN);
	}
	inline float intensity(float dist) const {
		return lum / std::pow(dist, 2);
	}
	inline float distance(float intensity) const {
		return std::sqrt(lum / intensity);
	}
	/* should a light ray be cast from this point to the light(ie: is the point close enough to be illuminated by the light)
	a small random value is added because otherwise many identical lights in one location can cause crisp falloff lines
	this could be avoided by increasing the casting radius but that increases light rays cast per light source polynomially */
	inline bool shouldCastToPoint(const Vec3 &point) const {
		int x = (int)point.axis[AXIS_X] - (int)pos.axis[AXIS_X];
		int y = (int)point.axis[AXIS_Y] - (int)pos.axis[AXIS_Y];
		int z = (int)point.axis[AXIS_Z] - (int)pos.axis[AXIS_Z];
		x = std::abs(x);
		y = std::abs(y);
		z = std::abs(z);
		unsigned int rad = (x + y + z);
		unsigned int rand = std::hash<unsigned int>()(rad) ^ std::hash<unsigned int>()(omp_get_thread_num());
		return rad < radius + (rand & LIGHT_CAST_RAND_ADD);
	}
};

#endif
