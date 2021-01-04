#ifndef VERT
#define VERT

#include "vec3.h"

// Mesh vertex
class Vert {
public:
	Vec3 pos;
	Vert () {}
	Vert (Vec3 pos) : pos(pos) {}
};

#endif
