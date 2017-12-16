#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#define STB_IMAGE_IMPLEMENTATION
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <vector>
#include <algorithm>
#include "Matrix.h"
#include "ShaderProgram.h"
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else		
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define WIDTH 1280
#define HEIGHT 720
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

//SHADER PROGRAM & MATRICES
class Entity;
SDL_Window* displayWindow;
ShaderProgram * program;
Matrix projectionMatrix, modelviewMatrix;

//TEXTURES & MEDIA
GLuint Textexture;
enum EntityType { PLAYER, MOB, PBULLET, EBULLET, BG, START };
enum GameState { MENU, GAME };
std::vector<Entity *> enemies, playerBullets, enemyBullets;
Entity * player;
Entity * gameBackground;
Mix_Music * music;

//GAME LOGIC
int initialDifficulty, state, leftMob, rightMob, enemiesAlive;
bool hit, lose, playerMoving, moveEnemies, canShoot;
float rightMax, leftMax, shootCooldown;

//FUNCTIONS IMPORTED FROM SLIDES
class Vector2 {
public:
	Vector2() {}
	Vector2(float a, float b) : x(a), y(b) {}
	float x;
	float y;
};

class Vector3 {
public:
	Vector3() {}
	Vector3(float a, float b, float c) : x(a), y(b), z(c) {}
	float x;
	float y;
	float z;
};

class Vector4 {
public:
	Vector4() {}
	Vector4(float a, float b, float c, float d) : l(a), t(b), b(c), r(d) {}
	float l;
	float t;
	float b;
	float r;
};

void DrawText(ShaderProgram *program, std::string text, float size, float spacing) {
	float texture_size = 1.0 / 16.0f;
	std::vector<float> vertexData;
	std::vector<float> texCoordData;

	for (size_t i = 0; i < text.size(); i++) {
		int spriteIndex = (int)text[i];
		float texture_x = (float)(spriteIndex % 16) / 16.0f;
		float texture_y = (float)(spriteIndex / 16) / 16.0f;
		vertexData.insert(vertexData.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
		});
		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x + texture_size, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x, texture_y + texture_size,
		});
	}
	glUseProgram(program->programID);
	glBindTexture(GL_TEXTURE_2D, Textexture);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6); //6 coordinates * nnumbers of letters

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

GLuint LoadTexture(const char *filePath) {
	int width, height, comp;
	unsigned char* image = stbi_load(filePath, &width, &height, &comp, STBI_rgb_alpha);
	if (image == NULL)
		std::cout << "Image did not load." << std::endl;

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(image);
	return texture;
}

class SheetSprite {
public:
	GLuint textureID;
	float u, v, width, height;
	Vector2 size;
	SheetSprite() {};
	SheetSprite(GLuint textureID, float u, float v, float width, float height, Vector2 size) :
		textureID(textureID), u(u), v(v), width(width), height(height), size(size) {};
	void draw() {
		glBindTexture(GL_TEXTURE_2D, textureID);
		float texCoords[] = {
			u, v + height,
			u + width, v,
			u, v,
			u + width, v,
			u, v + height,
			u + width, v + height
		};
		float aspect = width / height;
		float vertices[] = {
			-0.5f * size.x * aspect, -0.5f * size.y,
			0.5f * size.x * aspect, 0.5f * size.y,
			-0.5f * size.x * aspect, 0.5f * size.y,
			0.5f * size.x * aspect, 0.5f * size.y,
			-0.5f * size.x * aspect, -0.5f * size.y,
			0.5f * size.x * aspect, -0.5f * size.y
		};
		glUseProgram(program->programID);

		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program->positionAttribute);

		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program->texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program->positionAttribute);
		glDisableVertexAttribArray(program->texCoordAttribute);
	};
};

class Entity {
public:
	Matrix movement;
	Vector4 dimensions;
	Vector2 position, size, velocity;
	GLuint Gtexture, pTexture;
	SheetSprite sprite;
	EntityType type;
	bool alive;
	float difficulty, elapsed, enemyShootTime;
	Mix_Chunk * playerShot;
	Mix_Chunk * enemyShot;
	Entity(EntityType newType, float x, float y, float width, float height, Vector2 newVel = Vector2(0, 0)) {
		type = newType;
		velocity.x = newVel.x;
		velocity.y = newVel.y;

		size.x = width;
		size.y = height;

		position.x = x;
		position.y = y;

		dimensions.b = position.y - (size.y / 2);
		dimensions.t = position.y + (size.y / 2);
		dimensions.l = position.x - (size.x / 2);
		dimensions.r = position.x + (size.x / 2);

		alive = true;

		//DATA INSTANTIATION BY GAME TYPE
		if (type == EntityType::MOB) {
			Gtexture = LoadTexture("sprites/invaders.png"); //46 x 32, 1: 22 x 16, 2: 24 x 16
			Vector4 coords;
			coords.l = ((rand() % 2) * 22.f) / 46.f;
			coords.r = ((rand() % 2) * 16.f) / 32.f;
			coords.t = 16.f / 32.f;	//coords.t is height, coords.b is width

			if (coords.l > 0) {
				coords.b = 24.f / 46.f;
			}else {
				coords.b = 22.f / 46.f;
			}

			elapsed = 0;
			enemyShootTime = rand() % 20 + 1;
			std::cout << coords.l << " " << coords.r << " " << coords.b << " " << coords.t << std::endl;

			sprite = SheetSprite(Gtexture, coords.l, coords.r, coords.b, coords.t, size);
			enemyShot = Mix_LoadWAV("sounds/enemyShot.wav");
			difficulty = initialDifficulty;
		}
		else if (type == EntityType::PLAYER) {
			pTexture = LoadTexture("sprites/player.png");
			sprite = SheetSprite(pTexture, 0.f, 0.f, 1.f, 1.f, size);
			playerShot = Mix_LoadWAV("sounds/playerShot.wav");

			difficulty = 1;
			playerMoving = false;
			hit = false;
			lose = false;
		}
		else if (type == EntityType::PBULLET) {
			Gtexture = LoadTexture("sprites/laser_player.png");
			sprite = SheetSprite(Gtexture, 0.f, 0.f, 1.f, 1.f, size);

			difficulty = 1;
		}else if (type == EntityType::EBULLET) {
			Gtexture = LoadTexture("sprites/laser_enemy.png");
			sprite = SheetSprite(Gtexture, 0.f, 0.f, 1.f, 1.f, size);

			difficulty = 1;
		}
		else if (type == EntityType::BG) {
			Gtexture = LoadTexture("sprites/bg.jpg");
			sprite = SheetSprite(Gtexture, 0.f, 0.f, 1.f, 1.f, size);
		}
	}

	void update(float time) {
		if (alive) {
			//DISPLACEMENT 
			position.x += velocity.x * difficulty * time;
			position.y += velocity.y * difficulty * time;

			if (type == EntityType::MOB) {
				elapsed += time;

				if (elapsed > enemyShootTime) {
					elapsed = 0;
					enemyShootTime = rand() % 20 + 3;
					Mix_PlayChannel(1, enemyShot, 0);
					enemyBullets.push_back(new Entity(EntityType::EBULLET, position.x, dimensions.b, 0.04f, 0.16f, Vector2(0, -1.7f)));
				}
				difficulty = initialDifficulty - enemiesAlive + 1;

				for (Entity * bullet : playerBullets) {
					if (collideWith(*bullet)) {
						alive = false;
						bullet->alive = false;
					}
				}
			}

			if (type == EntityType::PLAYER) {
				//COLLISIONS, LIVES, GAME LOGIC
				for (Entity * obstacle : enemies) {
					if (collideWith(*obstacle)) {
						lose = true;
						alive = false;
					}
				}

				for (Entity * bullet : enemyBullets) {
					if (collideWith(*bullet)) {
						lose = true;
						alive = false;
					}
				}
			}

			dimensions.b = position.y - (size.y / 2);
			dimensions.t = position.y + (size.y / 2);
			dimensions.l = position.x - (size.x / 2);
			dimensions.r = position.x + (size.x / 2);
		}
	}

	void draw() {
		if (alive) {
			movement.Identity();
			movement.Translate(position.x, position.y, 0);
			program->SetModelviewMatrix(movement);
			sprite.draw();
		}
	}

	bool collideWith(Entity& other) {
		if (!(dimensions.b > other.dimensions.t ||
			dimensions.t < other.dimensions.b ||
			dimensions.r < other.dimensions.l ||
			dimensions.l > other.dimensions.r)) {
			return true;
		}
		return false;
	}
};

bool shouldRemoveBullet(Entity * bullet) {
	if (bullet->position.y < -2.f || bullet->position.y > 2.f)
		return true;
	else if (!bullet->alive)
		return true;
	return false;
}

void updateMobMovements() {
	enemiesAlive = 0;
	for (int i = 0; i < enemies.size(); i++) {
		if (enemies[i]->alive)
			enemiesAlive++;
		if (enemies[i]->dimensions.r >= rightMax) {
			rightMob = i;
			rightMax = enemies[i]->dimensions.r;
		}
		if (enemies[i]->dimensions.l <= leftMax) {
			leftMob = i;
			leftMax = enemies[i]->dimensions.l;
		}

		if (enemies[rightMob]->dimensions.r >= 3.55f) {
			enemies[i]->velocity.x = -fabs(enemies[i]->velocity.x);
			moveEnemies = true;
		}
		else if (enemies[leftMob]->dimensions.l <= -3.55f) {
			enemies[i]->velocity.x = fabs(enemies[i]->velocity.x);
			moveEnemies = true;
		}
	}

	if (moveEnemies) {
		for (Entity * enemy : enemies)	//THERE IS A SLIGHT OPTIMIZATION ISSUE HERE (DELAY)
			enemy->position.y -= 0.15f;
		moveEnemies = false;
	}
}

void updateGame(float elapsed) {
	player->update(elapsed);
	updateMobMovements();
	for (int i = 0; i < enemies.size(); i++)
		enemies[i]->update(elapsed);

	playerBullets.erase(std::remove_if(playerBullets.begin(), playerBullets.end(), shouldRemoveBullet), playerBullets.end());
	enemyBullets.erase(std::remove_if(enemyBullets.begin(), enemyBullets.end(), shouldRemoveBullet), enemyBullets.end());

	for (Entity * bullet : playerBullets) 
		bullet->update(elapsed);
	for (Entity * bullet : enemyBullets) 
		bullet->update(elapsed);

	shootCooldown += elapsed;
	if (shootCooldown >= 0.5f) {
		shootCooldown = 0;
		canShoot = true;
	}
}

void drawMenu() {
	glClearColor(0.f, 0.f, 0.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	modelviewMatrix.Identity();
	modelviewMatrix.Translate(-3.f, 1.5f, 0.f);
	program->SetModelviewMatrix(modelviewMatrix);
	DrawText(program, "SPACE INVADERS", 0.4f, -.1f);

	modelviewMatrix.Identity();
	modelviewMatrix.Translate(-3.f, -1.2f, 0.f);
	program->SetModelviewMatrix(modelviewMatrix);
	DrawText(program, "Press ENTER to begin!", 0.2f, -.08f);
}

void drawGame() {
	glClearColor(0.f, 0.f, 0.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	gameBackground->draw();

	for (Entity * enemy : enemies) 
		enemy->draw();
	for (Entity * bullet : playerBullets)
		bullet->draw();
	for (Entity * bullet : enemyBullets)
		bullet->draw();
	player->draw();
}

void Update(float elapsed) {
	switch (state) {
	case MENU:
		break;
	case GAME:
		updateGame(elapsed);
		break;
	}
}

void Draw() {
	switch (state) {
	case MENU:
		drawMenu();
		break;
	case GAME:
		drawGame();
		break;
	}
}

void Clean() {	//DELETES ALL POINTER DATA WHEN GAME IS CLOSED
	Mix_FreeChunk(player->playerShot);
	delete(player);
	delete(gameBackground);
	Mix_FreeMusic(music);
	for (int i = 0; i < enemies.size(); i++) {
		Mix_FreeChunk(enemies[i]->enemyShot);
		delete enemies[i];
	}
}

int main(int argc, char *argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("SPACE INVADERS", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif
	glViewport(0, 0, WIDTH, HEIGHT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	program = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

	projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
	program->SetModelviewMatrix(modelviewMatrix);
	program->SetProjectionMatrix(projectionMatrix);

	//GAME MANAGER & TIME STEP
	state = 0;
	float elapsed;
	float lastFrameTicks = 0.0f;

	//GAME SETUP & OBJECT INSTANTIATION
	gameBackground = new Entity(EntityType::BG, 0, 0, 7.1f, 4.0f);
	Textexture = LoadTexture("sprites/text.png");

	float enemySize = 0.2f;
	int maxEnemies = 30;
	initialDifficulty = maxEnemies;

	for (int w = 0; w < int(sqrt(maxEnemies)); w++) {
		for (int h = 0; h < int(sqrt(maxEnemies)); h++)
			enemies.push_back(new Entity(EntityType::MOB, -3.f + 2 * enemySize * w, 1.8f - 2 * enemySize * h, 
				enemySize, enemySize, Vector2(0.1f, 0)));
	}
	leftMob = 0;
	leftMax = enemies[leftMob]->dimensions.l;
	rightMob = enemies.size() / 2;
	rightMax = enemies[rightMob]->dimensions.r;

	player = new Entity(EntityType::PLAYER, 0.f, -1.5f, 0.5f, 0.5f, Vector2(0.0f, 0.0f));
	canShoot = true;
	shootCooldown = 0;
	
	//MUSIC (I LIKE KIRBY MUSIC)
	music = Mix_LoadMUS("sounds/spaceInvaders.mp3");
	Mix_PlayMusic(music, -1);

	SDL_Event event;
	bool done = false;
	while (!done) {
		// KEYBOARD INPUTS
		const Uint8 *keys = SDL_GetKeyboardState(NULL);
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				//Clean();
				done = true;
			}
			else if (event.type == SDL_KEYDOWN) {
				if (keys[SDL_SCANCODE_LEFT]) {
					player->velocity.x = -2.f;
					playerMoving = true;
				}
				else if (keys[SDL_SCANCODE_RIGHT]) {
					player->velocity.x = 2.f;
					playerMoving = true;
				}
				else if (keys[SDL_SCANCODE_RETURN]) {
					if (state != 1) {
						state = 1;
					}
				}
			}
			else if (event.type == SDL_KEYUP) {
				if (event.key.keysym.scancode == SDL_SCANCODE_LEFT || event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
					player->velocity = Vector2(0, 0);
					playerMoving = false;
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE && player->alive && canShoot) {
					playerBullets.push_back(new Entity(EntityType::PBULLET, player->position.x, player->dimensions.t, 0.04, 0.16f, Vector2(0, 2.0f)));
					Mix_PlayChannel(1, player->playerShot, 0);
					canShoot = false;
				}
			}
		}

		// TIME STEP
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		float fixedElapsed = elapsed;
		if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS) {
			fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
		}
		while (fixedElapsed >= FIXED_TIMESTEP) {
			fixedElapsed -= FIXED_TIMESTEP;
		}

		//UPDATE
		Update(elapsed);

		//DRAW
		Draw();

		SDL_GL_SwapWindow(displayWindow);
	}
	SDL_Quit();
	return 0;
}
