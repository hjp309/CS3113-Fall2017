#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <cassert>
#include <vector>
#include "Matrix.h"
#include "Entity.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else		
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

SDL_Window* displayWindow;
float windowWidth = 1024;
float windowHeight = 576;

//Objects
Entity player(EntityType::PLAYER, -2, 0);
Entity flag(EntityType::FLAG, 0, -1);
std::vector<Entity *> background;
std::vector<Entity *> platform;
Matrix projectionMatrix;
Matrix modelviewMatrix;

void Update(float elapsed) {
	float penetration = 0;
	player.update(elapsed);
	if (player.position.x > 2 || player.position.x < -2) {
		projectionMatrix.Translate(-player.velocity.x * elapsed, 0, 0);
	}
	if (player.position.y - 0.75 <= -2) {
		//float penetration = fabs(player.position.y - 2 - (player.height / 2.0f) - (-2)) + 0.00001f;
		player.velocity.y = 0;
	}

	/*for (size_t i = 0; i < platform.size(); i++) {
		if (player.collideWith(*platform[i])) {
			float difference = fabs(player.position.y - platform[i]->position.y);
			penetration = fabs(difference - (player.height/2.0f) - (platform[i]->height/2.0f)) + 0.00001f;
			if (player.position.y > platform[i]->position.y) {
				player.position.y += penetration;
				player.side.b += penetration;
				player.side.t += penetration;
				player.collidedBottom = true;
				player.velocity.y = 0.0f;
			}
			else {
				player.position.y -= penetration;
				player.side.b -= penetration;
				player.side.t -= penetration;
				player.collidedTop = true;
				player.velocity.y = 0;
			}
		}
	}*/

	if (player.collideWith(flag)) {
		std::cout << "COLLIDED!" << std::endl;
		flag.consumed = true;
	}


}

int main(int argc, char *argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Hw01", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif
	float elapsed;
	float lastFrameTicks = 0;

	glViewport(0, 0, 1024, 576);

	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, 1.0f, 0.0f);

	//Object creation
	for (int i = 0; i < 15; i++) {
		platform.push_back(new Entity(EntityType::BLOCK, -3.55f + (0.5*0.5) + i * 0.5, -2 + (0.5*0.5)));
	}
	for (int y = 0; y < 15; y++) {
		for (int x = 0; x < 30; x++) {
			background.push_back(new Entity(EntityType::BG, -3.55f + (0.5*0.5) + x * 0.5, -2 + (0.5*0.5) + y * 0.5, false));
		}
	}

	SDL_Event event;
	bool done = false;
	while (!done) {
		//Event handler
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			else if (event.type == SDL_KEYDOWN){
				if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
					player.faceRight = false;
					player.velocity.x = 5;
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
					player.faceRight = true;
					player.velocity.x = 5;
				}
			}
			else if (event.type == SDL_KEYUP) {
				if (event.key.keysym.scancode == SDL_SCANCODE_LEFT || event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
					player.velocity.x = 0;
				}
			}
		}
		const Uint8 *keys = SDL_GetKeyboardState(NULL);

		//Time Optimization
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		float fixedElapsed = elapsed;
		if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS) {
			fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
		}
		while (fixedElapsed >= FIXED_TIMESTEP) {
			fixedElapsed -= FIXED_TIMESTEP;
			Update(FIXED_TIMESTEP);
		}

		//Drawing
		glClearColor(0.4f, 0.2f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(program.programID);
		program.SetProjectionMatrix(projectionMatrix);
		program.SetModelviewMatrix(modelviewMatrix);

		for (Entity * b : background)
			b->draw(&program);
		for(Entity * p: platform)
			p->draw(&program);
		player.draw(&program);
		if(!flag.consumed)
			flag.draw(&program);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
