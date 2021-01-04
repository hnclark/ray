#ifndef MODEL
#define MODEL

#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <limits>
#include <cmath>

#include <string.h>

#include "common.h"
#include "vec3.h"
#include "ray.h"
#include "bbox.h"
#include "vert.h"
#include "tri.h"
#include "octree.h"

class CacheEntry {
public:
	Vec3 dir;
	const Tri *tri;
	bool valid;
	CacheEntry () : valid(false) {}
};

class ModelRayCache {
public:
	Vec3i dim;
	Vec3 offset;
	CacheEntry *x_minus, *x_plus, *y_minus, *y_plus, *z_minus, *z_plus;
	bool allocated;

	ModelRayCache () : allocated(false) {}
	#define CACHE_MARGIN Vec3(1,1,1)
	void allocate(const Vec3 &size, const Vec3 &_offset) {
		dim = Vec3i(Vec3::add(size, Vec3::scale(CACHE_MARGIN, 2)));
		offset = Vec3::sub(_offset, CACHE_MARGIN);

		// it may seem funny to allocate a cache for the bottom of an object in a top-down engine
		// but remember that this is the bottom of the object, not any particular instance of it
		// if an instance was rotated sideways or upside down, rays would impact it there
		x_minus = new CacheEntry[dim.axis[AXIS_Y] * dim.axis[AXIS_Z]];
		x_plus = new CacheEntry[dim.axis[AXIS_Y] * dim.axis[AXIS_Z]];
		y_minus = new CacheEntry[dim.axis[AXIS_X] * dim.axis[AXIS_Z]];
		y_plus = new CacheEntry[dim.axis[AXIS_X] * dim.axis[AXIS_Z]];
		z_minus = new CacheEntry[dim.axis[AXIS_X] * dim.axis[AXIS_Y]];
		z_plus = new CacheEntry[dim.axis[AXIS_X] * dim.axis[AXIS_Y]];

		float allocMB = (((2 * dim.axis[AXIS_Y] * dim.axis[AXIS_Z]) + (2 * dim.axis[AXIS_X] * dim.axis[AXIS_Z]) + (2 * dim.axis[AXIS_X] * dim.axis[AXIS_Y])) * sizeof(*z_plus)) / SIZE_MB;
		printf("allocated cache of dim (%d, %d, %d) - %.2f MB\n", dim.axis[AXIS_X], dim.axis[AXIS_Y], dim.axis[AXIS_Z], allocMB);

		allocated = true;
	}
	CacheEntry &index(const Ray &ray, int face, float bboxDist) {
		int axis1, axis2;
		CacheEntry *faceArray;

		switch(face) {
			case FACE_X_MIN:
				faceArray = x_minus;
				axis1 = AXIS_Y;
				axis2 = AXIS_Z;
				break;
			case FACE_X_PLUS:
				faceArray = x_plus;
				axis1 = AXIS_Y;
				axis2 = AXIS_Z;
				break;
			case FACE_Y_MIN:
				faceArray = y_minus;
				axis1 = AXIS_X;
				axis2 = AXIS_Z;
				break;
			case FACE_Y_PLUS:
				faceArray = y_plus;
				axis1 = AXIS_X;
				axis2 = AXIS_Z;
				break;
			case FACE_Z_MIN:
				faceArray = z_minus;
				axis1 = AXIS_X;
				axis2 = AXIS_Y;
				break;
			case FACE_Z_PLUS:
			default:
				faceArray = z_plus;
				axis1 = AXIS_X;
				axis2 = AXIS_Y;
				break;
		}
		
		int u = (int)std::fma(ray.dir.axis[axis1], bboxDist, ray.origin.axis[axis1] - offset.axis[axis1]);
		int v = (int)std::fma(ray.dir.axis[axis2], bboxDist, ray.origin.axis[axis2] - offset.axis[axis2]);
		int width = dim.axis[axis1];
		int height = dim.axis[axis2];

		return faceArray[ARRAY_INDEX(CLAMP(0, u, width - 1), CLAMP(0, v, height - 1), width)];
	}
	float lookup(Ray &ray, int face, float bboxDist, float precision) {
		float returned = RAY_INVALID;
		CacheEntry &cacheHit = index(ray, face, bboxDist);

		if (cacheHit.valid && Vec3::eq(ray.dir, cacheHit.dir, precision)) {
			if (cacheHit.tri != nullptr) {
				float depth = cacheHit.tri->rayCast(ray);
				if (depth != RAY_MISS) {
					return depth;
				} else {
					return RAY_INVALID;
				}
			} else {
				return RAY_MISS;
			}
		}
		return returned;
	}
	void set(const Ray &ray, int face, float bboxDist, bool setMiss = false) {
		CacheEntry &cacheHit = index(ray, face, bboxDist);

		cacheHit.dir = ray.dir;
		cacheHit.valid = true;
		if (!setMiss) {
			cacheHit.tri = ray.tri;
		} else {
			cacheHit.tri = nullptr;
		}
	}
};

// 3D Model
class Model {
private:
	ModelRayCache cache;
	OctNode *octree;

	void calcBBox() {
		if (tris.size() > 0) {
			Vec3 min = Vec3(tris[0]->bbox.min.axis[AXIS_X], tris[0]->bbox.min.axis[AXIS_Y], tris[0]->bbox.min.axis[AXIS_Z]);
			Vec3 max = Vec3(tris[0]->bbox.max.axis[AXIS_X], tris[0]->bbox.max.axis[AXIS_Y], tris[0]->bbox.max.axis[AXIS_Z]);
			BBox localbbox = BBox(min, max);
			int size = tris.size();

			#pragma omp declare reduction (+: BBox: omp_out += omp_in) initializer(omp_priv=BBox(omp_orig))
			#pragma omp parallel for reduction(+:localbbox)
			for (int i = 0; i < size; ++i) {
				localbbox += tris[i]->bbox;
			}
			bbox = localbbox;
		}
	}

public:
	std::vector<Vert *> verts;
	std::vector<Tri *> tris;
	BBox bbox;

	#define OBJ_PIXELS_PER_UNIT 100
	#define OBJ_UNITS_TO_PIXELS(units) (units * OBJ_PIXELS_PER_UNIT)

	#define MODEL_LOAD_LINE_BUFFER 512
	#define OBJ_PREFIX_SIZE 2
	#define OBJ_PREFIX_VERTEX "v "
	#define OBJ_PREFIX_VERTEX_NORMAL "vn"
	#define OBJ_PREFIX_VERTEX_TEXTURE "vt"
	#define OBJ_PREFIX_FACE "f "
	Model (const std::string &filename, bool cached) {
		char lineBuffer[MODEL_LOAD_LINE_BUFFER];
		std::ifstream file(filename);
		int vCount = 0, vNormalCount = 0, vTextureCount = 0, fCount = 0;
		while (file) {
			file.getline(lineBuffer, MODEL_LOAD_LINE_BUFFER);
			if (strncmp(OBJ_PREFIX_VERTEX, lineBuffer, OBJ_PREFIX_SIZE) == 0) {
				float x, y, z;
				sscanf(lineBuffer, "v %f %f %f", &x, &y, &z);
				/*
				models are scaled by a factor of 100 and the y axis is flipped, 
				this is so opening them in Blender will present them in the same orientation as the engine
				and so they won't clip in it
				*/
				verts.push_back(new Vert(Vec3(OBJ_UNITS_TO_PIXELS(x), OBJ_UNITS_TO_PIXELS(-1 * y), OBJ_UNITS_TO_PIXELS(z))));
				++vCount;
			} else if (strncmp(OBJ_PREFIX_VERTEX_NORMAL, lineBuffer, OBJ_PREFIX_SIZE) == 0) {
				// to handle vertex normals, just set verts[vNormalCount].normal here
				++vNormalCount;
			} else if (strncmp(OBJ_PREFIX_VERTEX_TEXTURE, lineBuffer, OBJ_PREFIX_SIZE) == 0) {
				// to handle vertex texture coords, just set verts[vTextureCount].texture here
				++vTextureCount;
			} else if (strncmp(OBJ_PREFIX_FACE, lineBuffer, OBJ_PREFIX_SIZE) == 0) {
				int v1, v2, v3;
				sscanf(lineBuffer, "f %d/%*s %d/%*s %d/%*s", &v1, &v2, &v3);
				// haha quirky off-by-one numbering system
				tris.push_back(new Tri(verts[v1 - 1], verts[v2 - 1], verts[v3 - 1]));
				++fCount;
			}
		}

		printf("Loaded model \"%s\", %ld verts, %ld tris\n", filename.c_str(), verts.size(), tris.size());
		calcBBox();
		printf("\tBBox: min(%f %f %f), max(%f %f %f)\n", bbox.min.axis[AXIS_X], bbox.min.axis[AXIS_Y], bbox.min.axis[AXIS_Z], bbox.max.axis[AXIS_X], bbox.max.axis[AXIS_Y], bbox.max.axis[AXIS_Z]);
		int octreeNodesRequired = tris.size() * OCTREE_NODES_PER_TRI;
		int octreeDepth = std::min((int)std::round(std::log(octreeNodesRequired) / std::log(8)), OCTREE_DEPTH_MAX);
		octree = Octree::calcOctree(bbox, tris, octreeDepth);
		printf("\tOctree: depth %d\n", octreeDepth);

		if (cached) {
			printf("\tCache: ");
			cache.allocate(Vec3::sub(bbox.max, bbox.min), bbox.min);
		}
	}

	float rayCast(Ray &ray, float targetDepth = RAY_MISS, bool shadowRay = false) {
		// check if the ray intersects the model's bounding box, if not, return false
		// then raycast using octree acceleration
		int face;
		float bboxDist = bbox.rayCastFace(ray, face);
		if (shadowRay && bboxDist > targetDepth) {
			return RAY_MISS;
		}
		if (bboxDist != RAY_MISS) {
			// cache lookup
			float lookup = RAY_INVALID;
			if (!shadowRay && cache.allocated) {
				lookup = cache.lookup(ray, face, bboxDist, 0.001);
			}

			float depth = RAY_MISS;
			if (lookup == RAY_INVALID) {
				depth = Octree::rayCastOctree(octree, ray, targetDepth, shadowRay);
				// cache set
				if (!shadowRay && cache.allocated) {
					cache.set(ray, face, bboxDist, depth == RAY_MISS);
				}
			} else {
				depth = lookup;
			}

			return depth;
		}
		return RAY_MISS;
	}
};

class ModelInstance {
public:
	Model *model;
	Vec3 pos;

	ModelInstance () {}
	ModelInstance (Model *model, Vec3 pos) : model(model), pos(pos) {}
	float rayCast(Ray &ray, float targetDepth = RAY_MISS, bool shadowRay = false) const {
		// offset ray to account for instance position
		Ray subRay = ray;
		Ray::translate(subRay, Vec3::scale(pos, -1));

		float depth = model->rayCast(subRay, targetDepth, shadowRay);
		// ray intersection
		if (depth != RAY_MISS) {
			// set all non-positional parameters
			Ray::setNonPos(ray, subRay);
		}
		return depth;
	}
};

#endif
