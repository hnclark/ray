#ifndef BBOX
#define BBOX

#include "common.h"
#include "vec3.h"
#include "ray.h"

// Bounding Box
class BBox {
public:
	Vec3 min, max;

	BBox () {}
	BBox (const Vec3 &a, const Vec3 &b) {
		min = Vec3(std::min(a.axis[AXIS_X], b.axis[AXIS_X]), std::min(a.axis[AXIS_Y], b.axis[AXIS_Y]), std::min(a.axis[AXIS_Z], b.axis[AXIS_Z]));
		max = Vec3(std::max(a.axis[AXIS_X], b.axis[AXIS_X]), std::max(a.axis[AXIS_Y], b.axis[AXIS_Y]), std::max(a.axis[AXIS_Z], b.axis[AXIS_Z]));
	}
	BBox (const Vec3 &v, float side) {
		float halfSide = side / 2;
		min = Vec3(v.axis[AXIS_X] - halfSide, v.axis[AXIS_Y] - halfSide, v.axis[AXIS_Z] - halfSide);
		max = Vec3(v.axis[AXIS_X] + halfSide, v.axis[AXIS_Y] + halfSide, v.axis[AXIS_Z] + halfSide);
	}
	BBox (const BBox &b) {
		min = b.min;
		max = b.max;
	}
	BBox &operator+=(const BBox &bbox) {
		if (bbox.min.axis[AXIS_X] < min.axis[AXIS_X]) {
			min.axis[AXIS_X] = bbox.min.axis[AXIS_X];
		} else if (bbox.max.axis[AXIS_X] > max.axis[AXIS_X]) {
			max.axis[AXIS_X] = bbox.max.axis[AXIS_X];
		}
		if (bbox.min.axis[AXIS_Y] < min.axis[AXIS_Y]) {
			min.axis[AXIS_Y] = bbox.min.axis[AXIS_Y];
		} else if (bbox.max.axis[AXIS_Y] > max.axis[AXIS_Y]) {
			max.axis[AXIS_Y] = bbox.max.axis[AXIS_Y];
		}
		if (bbox.min.axis[AXIS_Z] < min.axis[AXIS_Z]) {
			min.axis[AXIS_Z] = bbox.min.axis[AXIS_Z];
		} else if (bbox.max.axis[AXIS_Z] > max.axis[AXIS_Z]) {
			max.axis[AXIS_Z] = bbox.max.axis[AXIS_Z];
		}
		return *this;
	}
	inline bool containsPoint(const Vec3 &vec) const {
		return vec.axis[AXIS_X] > min.axis[AXIS_X] && vec.axis[AXIS_Y] > min.axis[AXIS_Y] && vec.axis[AXIS_Z] > min.axis[AXIS_Z] &&
			vec.axis[AXIS_X] < max.axis[AXIS_X] && vec.axis[AXIS_Y] < max.axis[AXIS_Y] && vec.axis[AXIS_Z] < max.axis[AXIS_Z];
	}
	inline float rayCast(const Ray &ray) const {
		float tmin = -RAY_MISS, tmax = RAY_MISS;

		for (int axis = 0; axis < AXIS_NUM; ++axis) {
			float t1 = (min.axis[axis] - ray.origin.axis[axis]) * ray.dir_inverse.axis[axis];
			float t2 = (max.axis[axis] - ray.origin.axis[axis]) * ray.dir_inverse.axis[axis];

			tmin = std::max(tmin, std::min(t1, t2));
			tmax = std::min(tmax, std::max(t1, t2));
		}

		if (tmax < tmin) {
			return RAY_MISS;
		} else {
			return tmin;
		}
	}
	inline float rayCastFace(const Ray &ray, int &face) const {
		face = FACE_NONE;
		float tmin = -RAY_MISS, tmax = RAY_MISS;

		for (int axis = 0; axis < AXIS_NUM; ++axis) {
			float t1 = (min.axis[axis] - ray.origin.axis[axis]) * ray.dir_inverse.axis[axis];
			float t2 = (max.axis[axis] - ray.origin.axis[axis]) * ray.dir_inverse.axis[axis];

			// if this axis check increases tmin
			if (std::min(t1, t2) > tmin) {
				face = AXIS_TO_FACE(axis);

				// if t2 is closer than t1, ie: the 'max' face is closer
				if (t2 < t1) {
					// select the positive face on this axis
					FACE_MIN_TO_PLUS(face);
				}
			}

			tmin = std::max(tmin, std::min(t1, t2));
			tmax = std::min(tmax, std::max(t1, t2));
		}

		if (tmax < tmin) {
			return RAY_MISS;
		} else {
			return tmin;
		}
	}
	static inline bool overlap(const BBox &a, const BBox &b) {
		return geqMargin(a.max.axis[AXIS_X], b.min.axis[AXIS_X]) && geqMargin(b.max.axis[AXIS_X], a.min.axis[AXIS_X]) &&
			geqMargin(a.max.axis[AXIS_Y], b.min.axis[AXIS_Y]) && geqMargin(b.max.axis[AXIS_Y], a.min.axis[AXIS_Y]) &&
			geqMargin(a.max.axis[AXIS_Z], b.min.axis[AXIS_Z]) && geqMargin(b.max.axis[AXIS_Z], a.min.axis[AXIS_Z]);
	}
};

#endif
