make: main.cpp include/common.h include/bbox.h include/octree.h include/model.h include/ray.h include/scene.h include/tri.h include/vec3.h include/vert.h include/light.h
	g++ main.cpp -Wall -fopenmp -lSDL2main -lSDL2 -O3 -o main
