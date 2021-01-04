// OctNode class for octrees
class OctNode {
public:
	std::vector<Tri *> tris;
	OctNode *subnodes[8];
	BBox bbox;
};

namespace Octree {
	#define OCTREE_NODES_PER_TRI 4
	#define OCTREE_DEPTH_MAX 10
	#define OCTREE_LEAF_TRIANGLES 20
	static OctNode *calcOctree (const BBox &bbox, const std::vector<Tri *> &tris, int depth) {
		if (tris.size() == 0) {
			return nullptr;
		}

		if ((tris.size() < OCTREE_LEAF_TRIANGLES) || depth < 0) {
			OctNode *nodeptr = new OctNode();
			nodeptr->bbox = bbox;
			nodeptr->tris = tris;
			for (int i = 0; i < 8; ++i) {
				nodeptr->subnodes[i] = nullptr;
			}
			return nodeptr;
		}

		std::vector<Tri *> triangleLists[8];

		BBox bboxes[8];
		float xHalfSize, yHalfSize, zHalfSize;
		for (int x = 0; x < 2; ++x) {
			for (int y = 0; y < 2; ++y) {
				for (int z = 0; z < 2; ++z) {
					xHalfSize = (bbox.max.axis[AXIS_X] - bbox.min.axis[AXIS_X]) / 2;
					yHalfSize = (bbox.max.axis[AXIS_Y] - bbox.min.axis[AXIS_Y]) / 2;
					zHalfSize = (bbox.max.axis[AXIS_Z] - bbox.min.axis[AXIS_Z]) / 2;
					Vec3 newMin(bbox.min.axis[AXIS_X] + (x * xHalfSize), bbox.min.axis[AXIS_Y] + (y * yHalfSize), bbox.min.axis[AXIS_Z] + (z * zHalfSize));
					Vec3 newMax(newMin.axis[AXIS_X] + xHalfSize, newMin.axis[AXIS_Y] + yHalfSize, newMin.axis[AXIS_Z] + zHalfSize);
					bboxes[(x * 4) + (y * 2) + z] = BBox(newMin, newMax);
				}
			}
		}

		for (Tri *tri: tris) {
			for (int i = 0; i < 8; ++i) {
				if (BBox::overlap(bboxes[i], tri->bbox)) {
					triangleLists[i].push_back(tri);
				}
			}
		}

		OctNode *nodeptr = new OctNode();
		nodeptr->bbox = bbox;
		#pragma omp parallel for
		for (int i = 0; i < 8; ++i) {
			nodeptr->subnodes[i] = calcOctree(bboxes[i], triangleLists[i], depth - 1);
		}

		return nodeptr;
	}

	static float rayCastOctree(const OctNode *curNode, Ray &ray, float targetDepth = RAY_MISS, bool shadowRay = false) {
		if (!curNode) {
			// empty node
			return RAY_MISS;
		} else if (curNode->tris.size() != 0) {
			// leaf node
			float depth = targetDepth;
			// iterate through all triangles
			for (Tri *tri: curNode->tris) {
				if (tri->rayCast(ray) < depth) {
					depth = ray.depth;
					if (shadowRay) {
						return depth;
					}
				}
			}
			return depth;
		} else {
			// non-leaf node
			// ray distance to each subnode bounding box
			struct subnodeDepth {
				int index;
				float depth;
			} subnodeDepthBuffer[8];
			int subnodeDepthBufferEntries = 0;

			// calculate ray distance to each subnode bounding box
			for (int i = 0; i < 8; ++i) {
				OctNode *nextNode = curNode->subnodes[i];
				if (nextNode) {
					float depth = nextNode->bbox.rayCast(ray);
					if (depth != RAY_MISS) {
						subnodeDepthBuffer[subnodeDepthBufferEntries].index = i;
						subnodeDepthBuffer[subnodeDepthBufferEntries].depth = depth;
						++subnodeDepthBufferEntries;
					}
				}
			}

			//sort subnode buffer by ray distance
			std::sort(subnodeDepthBuffer, subnodeDepthBuffer + subnodeDepthBufferEntries, [](const struct subnodeDepth &a, const struct subnodeDepth &b) {return a.depth < b.depth;});

			// iterate through subnodes in order, stopping on first ray intersection
			float depth = targetDepth;
			for (int i = 0; i < subnodeDepthBufferEntries; ++i) {
				// if we previously intersected a triangle closer than this bounding box begins, end iteration
				if (depth < subnodeDepthBuffer[i].depth) {
					break;
				}
				OctNode *nextNode = curNode->subnodes[subnodeDepthBuffer[i].index];
				float curDepth = rayCastOctree(nextNode, ray, targetDepth, shadowRay);
				if (curDepth < depth) {
					depth = curDepth;
				}
				if (shadowRay && depth < targetDepth) {
					return depth;
				}
			}

			return depth;
		}
	}
}
