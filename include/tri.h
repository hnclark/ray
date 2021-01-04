#ifndef TRI
#define TRI

#include "vec3.h"
#include "ray.h"
#include "vert.h"

// Triangle
class Tri {
private:
	void calcBBox () {
		Vec3 min;
		for (int axis = 0; axis < AXIS_NUM; ++axis) {
			min.axis[axis] = std::min(std::min(verts[0]->pos.axis[axis], verts[1]->pos.axis[axis]), verts[2]->pos.axis[axis]);
		}
		Vec3 max;
		for (int axis = 0; axis < AXIS_NUM; ++axis) {
			max.axis[axis] = std::max(std::max(verts[0]->pos.axis[axis], verts[1]->pos.axis[axis]), verts[2]->pos.axis[axis]);
		}
		bbox = BBox(min, max);
	}

	void calcNormal () {
		normal = Vec3::normalize(Vec3::cross(Vec3::sub(verts[1]->pos, verts[0]->pos), Vec3::sub(verts[2]->pos, verts[0]->pos)));
	}
public:
	Vert *verts[3];
	Vec3 normal;
	BBox bbox;

	Tri (Vert *a, Vert *b, Vert *c) {
		verts[0] = a;
		verts[1] = b;
		verts[2] = c;
		calcNormal();
		calcBBox();
	}

	float rayCast(Ray &ray) const {
		// first check if it intersects the bounding box, then do the math for tri intersection
		// if it intersected, modify the ray and return true
		// this is the only area that should modify the ray
		// ray intersects the bounding box
		if (bbox.rayCast(ray) != RAY_MISS) {
			float l_dot_n = Vec3::dot(ray.dir, normal);
			// ray is not parallel to the surface
			if (l_dot_n != 0) {
				Vec3 p0_minus_l0 = Vec3::sub(verts[0]->pos, ray.origin);
				float depth = Vec3::dot(p0_minus_l0, normal) / l_dot_n;
				// possible ray intersection with triangle plane is nearer than previous ones
				if (depth < ray.depth && depth >= 0) {
					Vec3 its = Vec3::add(ray.origin, Vec3::scale(ray.dir, depth));
					// check if intersection lies within tri boundaries
					if (geqMargin(Vec3::dot(Vec3::cross(Vec3::sub(verts[1]->pos, verts[0]->pos), Vec3::sub(its, verts[0]->pos)), normal), 0)) {
						if (geqMargin(Vec3::dot(Vec3::cross(Vec3::sub(verts[2]->pos, verts[1]->pos), Vec3::sub(its, verts[1]->pos)), normal), 0)) {
							if (geqMargin(Vec3::dot(Vec3::cross(Vec3::sub(verts[0]->pos, verts[2]->pos), Vec3::sub(its, verts[2]->pos)), normal), 0)) {
								// ray intersects tri closer than previous intersections, update it
								ray.meshInfo.normal = normal;
								ray.tri = this;
								ray.depth = depth;
								ray.meshInfo.diffuse = Vec3(1, 1, 1); //TODO set this from texture
								return depth;
							}
						}
					}
				}
			}
		}
		return RAY_MISS;
	}
};

#endif
