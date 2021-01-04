#include <chrono>
#include <cmath>

#include <SDL2/SDL.h>
#include <omp.h>
#include <stdio.h>

#include "include/common.h"
#include "include/vec3.h"
#include "include/model.h"
#include "include/light.h"
#include "include/scene.h"

int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window = SDL_CreateWindow(WINDOW_NAME, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_SHOWN);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_Texture *buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBX8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	// time loading
	printf("Loading...\n");
	std::chrono::high_resolution_clock::time_point startLoadTime = std::chrono::high_resolution_clock::now();

	// Load scene
	// TODO load the camera position/model list from file
	Camera camera(Vec3(800, 800, 1500));

	Model ball("models/ball.obj", true);
	ModelInstance ball1(&ball, Vec3(600, 500, 0));
	camera.scene.addModel(&ball1);
	float ballVel = 10;

	Model pillar("models/pillar.obj", true);
	ModelInstance pillars[6];
	for (int x = 0; x < 2; ++x) {
		for (int y = 0; y < 3; ++y) {
			pillars[(x * 3) + y] = ModelInstance(&pillar, Vec3(150 + x * 1600, 200 + y * 400, 0));
			camera.scene.addModel(&pillars[(x * 3) + y]);
		}
	}

	Light camLight(Vec3(1, 0.5, 0), 150000, Vec3(500, 500, 500), true);
	camera.scene.addLight(&camLight);
	Light light2(Vec3(0.2, 0.5, 1), 150000, Vec3(1300, 100, 600), true);
	camera.scene.addLight(&light2);
	// Done loading scene

	// time loading
	std::chrono::duration<double> duration = std::chrono::high_resolution_clock::now() - startLoadTime;
	double timePassed = duration.count();
	printf("Loaded in %.2fs\n", timePassed);

	bool running = true;
	int frames = 0;
	std::chrono::high_resolution_clock::time_point prevTime = std::chrono::high_resolution_clock::now();
	while (running) {
		// handle user input
		// polling must be done before key states are updated
		SDL_Event event;
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT) {
				running = false;
			} else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					running = false;
				}
			}
		}

		// basic camera panning for testing purposes
		int movespeed = 10;
		int xmov = 0, ymov = 0, zmov = 0;
		const uint8_t* currentKeyStates = SDL_GetKeyboardState(NULL);
		if(currentKeyStates[SDL_SCANCODE_W]){
                    ymov = -1;
                } else if(currentKeyStates[SDL_SCANCODE_S]){
                    ymov = 1;
                }
		if(currentKeyStates[SDL_SCANCODE_A]){
                    xmov = -1;
                } else if(currentKeyStates[SDL_SCANCODE_D]){
                    xmov = 1;
                }
		if(currentKeyStates[SDL_SCANCODE_E]){
                    zmov = -1;
                } else if(currentKeyStates[SDL_SCANCODE_Q]){
                    zmov = 1;
                }
		camera.pos.axis[AXIS_X] += xmov * movespeed;
		camera.pos.axis[AXIS_Y] += ymov * movespeed;
		camera.pos.axis[AXIS_Z] += zmov * movespeed;

		// light that follows camera
		camLight.pos.axis[AXIS_X] = camera.pos.axis[AXIS_X];
		camLight.pos.axis[AXIS_Y] = camera.pos.axis[AXIS_Y];

		// animated model
		if (ball1.pos.axis[AXIS_X] > 1200 || ball1.pos.axis[AXIS_X] < 600) {
			ballVel *= -1;
		}
		ball1.pos.axis[AXIS_X] += ballVel;

		// debug info
		std::chrono::high_resolution_clock::time_point curTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> duration = curTime - prevTime;
		double timePassed = duration.count();
		if (timePassed > PRINT_FPS_TIME) {
			printf("FPS: %.1f - %.2fs per frame\n", frames/timePassed, timePassed/frames);
			printf("\tx: %f y: %f z: %f\n", camera.pos.axis[AXIS_X], camera.pos.axis[AXIS_Y], camera.pos.axis[AXIS_Z]);

			prevTime = curTime;
			frames = 0;
		}
		++frames;

		// lock buffer for editing
		int pitch;
		uint32_t *pixels;
		SDL_LockTexture(buffer, NULL, (void **)&pixels, &pitch);

		// dynamically assign rows to threads
		#pragma omp parallel for schedule(dynamic)
		for (int y = 0; y < SCREEN_HEIGHT; ++y) {
			for (int x = 0; x < SCREEN_WIDTH; ++x) {
				pixels[ARRAY_INDEX(x,y,SCREEN_WIDTH)] = camera.renderPixel(x, y);
			}
		}

		// output buffer to screen
		SDL_UnlockTexture(buffer);
		SDL_RenderCopy(renderer, buffer, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyTexture(buffer);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

