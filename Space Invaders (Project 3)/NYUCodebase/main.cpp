#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_image.h>
#include <algorithm>
#include "Entity.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else		
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define WIDTH 1280
#define HEIGHT 720
#define FIXED_TIMESTEP 0.0166666f

SDL_Window* displayWindow;
Entity player(EntityType::PLAYER, 0.0f, -1.35f, 0.5f);
std::vector<Entity *> mobs;
std::vector<Entity *> missiles;

float mobScale = 0.3f;
bool fired = false;

void updateMissile(float elapsed) {
	if (fired) {
		missiles.push_back(new Entity(player.getPlayerPos().x));
		std::cout << missiles.size() << std::endl;
		fired = false;
	}
	
	for (size_t i = 0; i < missiles.size(); i++) {
		missiles[i]->position.y += missiles[i]->velocity * elapsed;
		missiles[i]->timeActive += elapsed;
		if (missiles[i]->timeActive > 1.0f)
			missiles.erase(missiles.begin() + i);
		/*for (size_t j = 0; j < mobs.size(); j++) {
			if (missiles[i]->collideWith(*mobs[j])) {
				missiles.erase(missiles.begin() + i);
				mobs.erase(mobs.begin() + j);
			}
		}*/
	}
}

void update(float time) {
	for (Entity * mob : mobs)
		mob->update(time);
	player.update(time);
	updateMissile(time);
}


int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif	
	glViewport(0, 0, WIDTH, HEIGHT);
	Matrix projectionMatrix;
	Matrix modelviewMatrix;
	projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);

	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	SDL_Event event;
	bool done = false;
	float elapsed;
	float accumulator = 0.0f;
	float lastFrameTicks = 0.0f;

	for (size_t x = 0; x < 12; x++)
		for (size_t y = 0; y < 5; y++)
			mobs.push_back(new Entity(EntityType::MOB, -3.55 + 0.5*mobScale + x*mobScale, 2 - 0.5*mobScale - 0.1f - y*mobScale, mobScale));

	for (size_t x = 0; x < 20; x++)
		missiles.push_back(new Entity(EntityType::MISSILE, 0.0f, 0.0f));

	program.SetModelviewMatrix(projectionMatrix);
	program.SetProjectionMatrix(projectionMatrix);

	while (!done) {
		const Uint8 *keys = SDL_GetKeyboardState(NULL);

		//Process Events
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			else if (event.type == SDL_KEYDOWN) {
				if (keys[SDL_SCANCODE_LEFT])
					player.velocity = -1.0f;
				else if (keys[SDL_SCANCODE_RIGHT])
					player.velocity = 1.0f;

				if (keys[SDL_SCANCODE_SPACE])
					fired = true;
			}
			else
				player.velocity = 0.0f;
		}

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		elapsed += accumulator;
		if (elapsed < FIXED_TIMESTEP) {
			accumulator = elapsed;
			continue;
		}

		while (elapsed >= FIXED_TIMESTEP) {
			update(FIXED_TIMESTEP);
			elapsed -= FIXED_TIMESTEP;
		}
		accumulator = elapsed;

		//Render
		glUseProgram(program.programID);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glClearColor(.5f, .5f, .5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		player.draw(&program);
		for (Entity * a : mobs)
			a->draw(&program);

		for (size_t i = 0; i < missiles.size(); i++)
			missiles[i]->draw(&program);

		SDL_GL_SwapWindow(displayWindow);

	}

	SDL_Quit();
	return 0;
}