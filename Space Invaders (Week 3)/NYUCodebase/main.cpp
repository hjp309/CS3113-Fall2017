#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include "Entity.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else		
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define WIDTH 640
#define HEIGHT 360
#define FIXED_TIMESTEP 0.0166666f

SDL_Window* displayWindow;

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif	

	glViewport(0, 0, WIDTH, HEIGHT);
	Matrix projectionMatrix;
	Matrix modelviewMatrix;
	projectionMatrix.SetOrthoProjection(-2.0f, 2.0f, -2.0f, 2.0f, -1.0f, 1.0f);
	
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	SDL_Event event;
	bool done = false;
	float accumulator = 0.0f;
	float lastTicks = 0.0f;

	EntityType type = PLAYER;
	EntityType mob1 = MOB1;

	Entity player(type, 0.0f, -3.0f);
	std::vector<Entity *> mobs;
	for(size_t x = 0; x < 4; x++)
		for (size_t y = 0; y < 4; y++) {
			mobs.push_back(new Entity(mob1, x, y));

		}

	const Uint8 *keys = SDL_GetKeyboardState(NULL);

	while (!done) {
		//Process Events
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		float ticks = float(SDL_GetTicks()) / 1000.0f;
		float elapsed = ticks - lastTicks;
		lastTicks = ticks;
		elapsed += accumulator;
		if (elapsed < FIXED_TIMESTEP) {
			accumulator = elapsed;
			continue;
		}

		glUseProgram(program.programID);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		program.SetModelviewMatrix(projectionMatrix);
		program.SetProjectionMatrix(projectionMatrix);

		//Update
		while (elapsed >= FIXED_TIMESTEP) {
			elapsed -= FIXED_TIMESTEP;
		}

		//Render
		glClearColor(.5f, .5f, .5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		player.draw(&program);
		for (Entity * a : mobs)
			a->draw(&program);

		SDL_GL_SwapWindow(displayWindow);

	}

	SDL_Quit();
	return 0;
}
