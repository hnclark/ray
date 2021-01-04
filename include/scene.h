#ifndef SCENE
#define SCENE

#include <vector>
#include <limits>

#include "common.h"
#include "vec3.h"
#include "ray.h"
#include "model.h"
#include "light.h"

// Scene
#define AMBIENT_LIGHT 0
class Scene {
public:
	std::vector<ModelInstance *> models;
	std::vector<Light *> lights;
	float ambientLight;

	Scene () : ambientLight(AMBIENT_LIGHT) {}
	void addModel(ModelInstance *model) {
		models.push_back(model);
	}
	void addLight(Light *light) {
		lights.push_back(light);
	}
	float rayCast(Ray &ray, float targetDepth = RAY_MISS, bool shadowRay = false) const {
		float depth = targetDepth;
		for (ModelInstance *model: models) {
			float newDepth = model->rayCast(ray, targetDepth, shadowRay);
			if (newDepth < depth) {
				depth = newDepth;
			}
			if (shadowRay && leqMargin(depth, targetDepth)) {
				return depth;
			}
		}
		return depth;
	}
	uint32_t renderRay(Ray &ray) const {
		// geometry raycast
		float depth = rayCast(ray);

		// light raycast
		if (depth != RAY_MISS) {
			Vec3 its = Vec3::add(ray.origin, Vec3::scale(ray.dir, depth));

			float totalLum = AMBIENT_LIGHT;
			Vec3 avgColor(ray.meshInfo.diffuse);

			for (Light *light: lights) {
				if (light->shouldCastToPoint(its)) {
					Vec3 lightVec = Vec3::sub(its, light->pos);
					Ray lightRay(light->pos, lightVec);
					float lightRayLen = Vec3::lengthOf(lightVec);

					if ((!light->shadowCast || eqMargin(rayCast(lightRay, lightRayLen, true), lightRayLen))) {
						float sDot = std::max(Vec3::dot(ray.meshInfo.normal, lightRay.dir), 0.0f);
						float sIntensity = light->intensity(lightRayLen);
						float sLum = (sIntensity * sDot);
						Vec3::m_add(avgColor, Vec3::scale(light->color, sLum));
						totalLum += sLum;
					}
				}
			}

			Vec3::m_normalize(avgColor);
			Vec3::m_scale(avgColor, totalLum);
			Vec3::m_cap(avgColor, COLOR_MAX);
			return 0x00000000 | ((int)(avgColor.axis[COLOR_R] * 255) << 24) | ((int)(avgColor.axis[COLOR_G] * 255) << 16) | ((int)(avgColor.axis[COLOR_B] * 255) << 8);
		}

		return 0x1F1F1F00;
	}
};

// Camera
class Camera {
public:
	Scene scene;
	Vec3 pos;
	Camera () {}
	Camera (Vec3 pos) : pos(pos) {}
	uint32_t renderPixel(int x, int y) const {
		// Orthographic
		Ray ray(Vec3((int)pos.axis[AXIS_X] + x - (SCREEN_WIDTH / 2), (int)pos.axis[AXIS_Y] + y - (SCREEN_HEIGHT / 2), pos.axis[AXIS_Z]), Vec3(0.1, -0.2, -1));
		// Perspective
		//float focalLength = 1000;
		//Ray ray(Vec3(pos.axis[AXIS_X], pos.axis[AXIS_Y], pos.axis[AXIS_Z]), Vec3(x - (SCREEN_WIDTH / 2), y - (SCREEN_HEIGHT / 2), -1 * focalLength));
		return scene.renderRay(ray);
	}
};

#endif
