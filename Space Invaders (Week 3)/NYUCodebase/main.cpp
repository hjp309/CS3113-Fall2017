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

	Entity player(EntityType::PLAYER, 0.0f, -3.0f);
	std::vector<Entity *> mobs;
	for(size_t x = 0; x < 8; x++)
		for (size_t y = 0; y < 8; y++) {
			if (y == 0)
				mobs.push_back(new Entity(EntityType::MOB1, x * 1.2 - 2, y * 1.1));
			if (y == 1)
				mobs.push_back(new Entity(EntityType::MOB2, x * 1.2 - 2, y * 1.1));
			if (y == 2)
				mobs.push_back(new Entity(EntityType::MOB3, x * 1.2 - 2, y * 1.1));
			if (y == 3)
				mobs.push_back(new Entity(EntityType::MOB4, x * 1.2 - 2, y * 1.1));
		}

	float playerX;
	const Uint8 *keys = SDL_GetKeyboardState(NULL);

	while (!done) {
		float ticks = float(SDL_GetTicks()) / 1000.0f;
		float elapsed = ticks - lastTicks;
		lastTicks = ticks;
		elapsed += accumulator;
		if (elapsed < FIXED_TIMESTEP) {
			accumulator = elapsed;
			continue;
		}

		//Process Events
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;	
			}
			else if (event.type == SDL_KEYDOWN) {
				if (keys[SDL_SCANCODE_LEFT])
					playerX = -fabs(5.0f);
				else if (keys[SDL_SCANCODE_RIGHT])
					playerX = fabs(5.0f);
			}
			else
				playerX = 0;
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
		player.update(elapsed, playerX * elapsed, 0);

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
