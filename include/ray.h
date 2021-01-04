#ifndef RAY
#define RAY

#include "common.h"
#include "vec3.h"

class MeshInfo {
public:
	Vec3 normal, diffuse;
	MeshInfo () {}
};

class Tri;

// Casted ray
class Ray {
private:
	void calcInverse() {
		dir_inverse = Vec3(1.0 / dir.axis[AXIS_X], 1.0 / dir.axis[AXIS_Y], 1.0 / dir.axis[AXIS_Z]);
	}
public:
	// Set upon creation
	Vec3 origin, dir, dir_inverse;
	// Set upon intersection - SHOULD NOT CONTAIN ANY 3D POSITIONAL INFORMATION DERIVED FROM MESH/TRIANGLE POSITIONS - OBJECT SPACE != WORLD SPACE
	// Positional information derived from the position of the ray origin/direction is fine since that's translated between the two spaces
	MeshInfo meshInfo;
	const Tri *tri;
	float depth;

	Ray () : depth(RAY_INVALID) {}
	Ray(Vec3 origin, Vec3 dir) : origin(origin), depth(RAY_MISS) {
		this->dir = Vec3::normalize(dir);
		calcInverse();
	}
	static inline void setNonPos(Ray &ray, const Ray &source){
		// sets non positional parameters of ray from source
		// Useful when the output of a locally transformed ray needs to be passed on
		ray.meshInfo = source.meshInfo;
		ray.tri = source.tri;
		ray.depth = source.depth;
	}
	static inline void translate(Ray &ray, const Vec3 &offset) {
		ray.origin.axis[AXIS_X] += offset.axis[AXIS_X];
		ray.origin.axis[AXIS_Y] += offset.axis[AXIS_Y];
		ray.origin.axis[AXIS_Z] += offset.axis[AXIS_Z];
	}
};

#endif
