#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#define STB_IMAGE_IMPLEMENTATION
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <vector>
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
enum EntityType { PLAYER, OBSTACLES, BG, HUD, FLOOR, START, END };
enum GameState { MENU, GAME, GAMEOVER };
std::vector<Entity *> obstacles, floorTiles, liveSprites;
Entity * player;
Entity * startBackground;
Entity * gameBackground;
Entity * endBackground;
Mix_Music * music;

//GAME LOGIC
int maxDifficulty, lives, score, state;
std::stringstream scoreStream;
bool hit, playerMoving;

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
	GLuint Gtexture, pTexture, invWinTexture;
	SheetSprite sprite, invWinSprite;
	EntityType type;
	bool faceRight, alive;
	float elapsed, animationTime, invWin;
	int difficulty;
	Mix_Chunk * hitSound;
	Mix_Chunk * invWinSound;
	Mix_Chunk * walkingSound;
	Entity(EntityType newType, float x, float y, float width, float height, Vector2 newVel = Vector2(0, 0)) {
		type = newType;
		velocity.x = newVel.x;
		velocity.y = newVel.y;

		size.x = width;
		size.y = height;
		
		elapsed = 0;
		animationTime = 0;
		difficulty = 1;
		alive = true;

		//DATA INSTANTIATION BY GAME TYPE
		if (type == EntityType::OBSTACLES) {
			Gtexture = LoadTexture("sprites/dirtCenter.png");
			sprite = SheetSprite(Gtexture, 0.f, 0.f, 1.f, 1.f, size);
		}
		else if (type == EntityType::PLAYER) {
			pTexture = LoadTexture("sprites/player.png");
			invWinTexture = LoadTexture("sprites/IWsymbol.png");
			sprite = SheetSprite(pTexture, 0.f, 0.f, 53.f / 159.f, 63.f / 126.f, size); //53 x 63, 159 x 126

			faceRight = true;
			playerMoving = false;
			hit = false;
			lives = 3;
			invWin = 2.0f;

			walkingSound = Mix_LoadWAV("sounds/walking.wav");	//Channel 0
			hitSound = Mix_LoadWAV("sounds/bonk.wav");			//Channel 1
			invWinSound = Mix_LoadWAV("sounds/invWin.wav");		//Channel 2
		}
		else if (type == EntityType::BG) {
			Gtexture = LoadTexture("sprites/BG.png");
			sprite = SheetSprite(Gtexture, 0.f, 0.f, 1.f, 63.f / 189.f, size); //231 x 63
		}
		else if (type == EntityType::FLOOR) {
			int i = rand() % 3;
			if (i == 0)
				Gtexture = LoadTexture("sprites/floor.png");
			else if (i == 1)
				Gtexture = LoadTexture("sprites/floor_1.png");
			else
				Gtexture = LoadTexture("sprites/floor_2.png");
			sprite = SheetSprite(Gtexture, 0.f, 0.f, 1.f, 1.f, size); //231 x 63
		}
		else if (type == EntityType::HUD) {
			Gtexture = LoadTexture("sprites/life.png");
			sprite = SheetSprite(Gtexture, 0.f, 0.f, 1.f, 1.f, size);
		}
		else if (type == EntityType::START) {
			Gtexture = LoadTexture("sprites/startMenu.png");
			sprite = SheetSprite(Gtexture, 0.f, 0.f, 1.f, 1.f, size);
		}
		else if (type == EntityType::END) {
			Gtexture = LoadTexture("sprites/feelsbadman.png");
			sprite = SheetSprite(Gtexture, 0.f, 0.f, 1.f, 1.f, size);
		}

		position.x = x;
		position.y = y;

		dimensions.b = position.y - (size.y / 2);
		dimensions.t = position.y + (size.y / 2);
		dimensions.l = position.x - (size.x / 2);
		dimensions.r = position.x + (size.x / 2);
	}

	void update(float time) {
		if (alive) {
			if (type == EntityType::OBSTACLES) {
				elapsed += time;
				if (elapsed > 5.0f) { //Every 5 seconds, increase speed
					elapsed = 0.f;
					difficulty++;
					if (difficulty > maxDifficulty) {
						difficulty = maxDifficulty;
					}
				}
				int deadZone = -3.f - (rand() % (maxDifficulty - difficulty + 1));	//Randomize spawn
				if (position.y < deadZone) {	
					position.x = -3.55f + ((rand() % 20) * 7.1) / 20;
					position.y = 5.f;
				}
			}

			if (type == EntityType::PLAYER) {
				score++;
				scoreStream.str("");
				scoreStream << int(score / 20);

				//ANIMATION
				if (playerMoving) {	
					animationTime++;
					Mix_PlayChannel(0, walkingSound, -1);
				}
				else {
					animationTime = 0;
					Mix_Pause(0);
				}
				if (faceRight && alive) {
					sprite = SheetSprite(pTexture, 0.f + int(animationTime) * (53.f/159.f), 0.f, 53.f / 159.f, 63.f / 126.f, size); //53 x 63, 159 x 126
				}
				else if(!faceRight && alive){
					sprite = SheetSprite(pTexture, 0.f + int(animationTime) * (53.f / 159.f), 0.5f, 53.f / 159.f, 63.f / 126.f, size);
				}

				//COLLISIONS, LIVES, GAME LOGIC
				for (Entity * obstacle : obstacles) {
					if (collideWith(*obstacle) && !hit) {
						//std::cout << "You're hit!" << std::endl;
						hit = true;
						obstacle->position.x = -3.55f + ((rand() % 20) * 7.1) / 20;
						obstacle->position.y = 5.f;
						lives--;
						Mix_PlayChannel(1, hitSound, 0);
						if (lives <= 0) {
							lives = 0;
							position = Vector2(0.f, -1.25f);
							velocity.x = 0;
							alive = false;
							Mix_PauseMusic();
							state = 2;
						}
						else {
							Mix_PlayChannel(2, invWinSound, 0);
						}
					}
				}

				//INVINCIBILITY WINDOW
				if (hit) {
					elapsed += time;
					if (elapsed >= invWin) {
						elapsed = 0;
						hit = false;
					}
				}
			}

			//DISPLACEMENT 
			position.x += velocity.x * difficulty * time;
			position.y += velocity.y * difficulty * time;

			dimensions.b = position.y - (size.y / 2);
			dimensions.t = position.y + (size.y / 2);
			dimensions.l = position.x - (size.x / 2);
			dimensions.r = position.x + (size.x / 2);
		}
	}

	void draw() {
		movement.Identity();
		movement.Translate(position.x, position.y, 0);
		program->SetModelviewMatrix(movement);
		sprite.draw();

		if (type == EntityType::PLAYER && hit) {
			modelviewMatrix.Identity();
			modelviewMatrix.Translate(position.x - 0.4f, position.y + 0.3f, 0.0f);
			program->SetModelviewMatrix(modelviewMatrix);
			DrawText(program, "INVINCIBLE!", 0.15f, -0.07f);
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

void updateGame(float elapsed) {
	player->update(elapsed);
	for (int i = 0; i < obstacles.size(); i++)
		obstacles[i]->update(elapsed);
}

void drawMenu() {
	glClearColor(0.4f, 0.2f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	startBackground->draw();
	modelviewMatrix.Identity();
	modelviewMatrix.Translate(-3.f, 1.5f, 0.f);
	program->SetModelviewMatrix(modelviewMatrix);
	DrawText(program, "Man vs Bricks", 0.4f, -.1f);

	modelviewMatrix.Identity();
	modelviewMatrix.Translate(-3.f, -1.2f, 0.f);
	program->SetModelviewMatrix(modelviewMatrix);
	DrawText(program, "Press ENTER to begin!", 0.2f, -.08f);

	modelviewMatrix.Identity();
	modelviewMatrix.Translate(-3.f, -1.f, 0.f);
	program->SetModelviewMatrix(modelviewMatrix);
	DrawText(program, "USE LEFT AND RIGHT ARROW KEYS TO KEEP THE MAN SAFE!", 0.2f, -0.08f);
}

void drawGame() {
	glClearColor(0.f, 0.f, 0.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	gameBackground->draw();
	for (Entity * tile : floorTiles)
		tile->draw();
	for (Entity * obstacle : obstacles)
		obstacle->draw();
	for (int i = 0; i < lives; i++)
		liveSprites[i]->draw();
	player->draw();

	modelviewMatrix.Identity();
	modelviewMatrix.Translate(-3.35f, 1.85f, 0.0f);
	program->SetModelviewMatrix(modelviewMatrix);
	DrawText(program, "LIVES", 0.2f, -0.1f);

	modelviewMatrix.Identity();
	modelviewMatrix.Translate(3.f, 1.8f, 0.0f); 
	program->SetModelviewMatrix(modelviewMatrix);
	DrawText(program, "SCORE", 0.2f, -0.1f);

	modelviewMatrix.Identity();
	modelviewMatrix.Translate(3.f, 1.6f, 0.0f);
	program->SetModelviewMatrix(modelviewMatrix);
	DrawText(program, scoreStream.str(), 0.2f, -0.1f);
}

void drawEnd() {
	glClearColor(1.f, 1.f, 1.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	endBackground->draw();
	modelviewMatrix.Identity();
	modelviewMatrix.Translate(0.f, 0.f, 0.f);
	program->SetModelviewMatrix(modelviewMatrix);
	DrawText(program, "GAME OVER!", 0.2f, 0.1f);

	modelviewMatrix.Identity();
	modelviewMatrix.Translate(-.5f, 1.5f, 0.0f);
	program->SetModelviewMatrix(modelviewMatrix);
	DrawText(program, "Your score was: ", 0.35f, -0.1f);

	modelviewMatrix.Identity();
	modelviewMatrix.Translate(.5f, 1.f, 0.0f);
	program->SetModelviewMatrix(modelviewMatrix);
	DrawText(program, scoreStream.str(), 0.55f, -0.1f);

	modelviewMatrix.Identity();
	modelviewMatrix.Translate(.5f, -1.f, 0.f);
	program->SetModelviewMatrix(modelviewMatrix);
	DrawText(program, "Press ENTER to retry!", 0.2f, -0.1f);
}

void Update(float elapsed) {
	switch (state) {
	case MENU:
		break;
	case GAME:
		updateGame(elapsed);
		break;
	case GAMEOVER:
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
	case GAMEOVER:
		drawEnd();
		break;
	}
}

void Clean() {	//DELETES ALL POINTER DATA WHEN GAME IS CLOSED
	Mix_FreeChunk(player->hitSound);
	Mix_FreeChunk(player->invWinSound);
	Mix_FreeChunk(player->walkingSound);
	delete(player);
	state = 3;
	delete(startBackground);
	delete(gameBackground);
	delete(endBackground);
	Mix_FreeMusic(music);
	for (int i = 0; i < obstacles.size(); i++)
		delete obstacles[i];
	for (int i = 0; i < floorTiles.size(); i++)
		delete floorTiles[i];
	for (int i = 0; i < liveSprites.size(); i++)
		delete liveSprites[i];
}

int main(int argc, char *argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Man vs. Brick", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
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

	//START SETUP
	startBackground = new Entity(EntityType::START, 0, 0, 7.1f, 4.5f);

	//END SETUP
	endBackground = new Entity(EntityType::END, -2.f, 0, 2.5f, 4.0f);

	//GAME SETUP & OBJECT INSTANTIATION
	float obstacleSize = 0.25f;
	int obstaclesMax = 7;
	maxDifficulty = 5;
	Textexture = LoadTexture("sprites/text.png");

	for (int i = 0; i < obstaclesMax; i++) {
		float x = -3.55f + ((rand() % 20) * 7.1) / 20;
		std::cout << x << std::endl;
		obstacles.push_back(new Entity(EntityType::OBSTACLES, x, 5.0f, obstacleSize, obstacleSize, Vector2(0, -0.5f)));
	}
	for (int i = 0; i < 20; i++) 
		floorTiles.push_back(new Entity(EntityType::FLOOR, -3.3f + 0.5f * i, -1.75f, 0.5f, 0.5f));
	for (int i = 0; i < 3; i++)
		liveSprites.push_back(new Entity(EntityType::HUD, -3.3f + 0.25f * i, 1.6f, 0.25f, 0.25f));
	player = new Entity(EntityType::PLAYER, 0.f, -1.25f, 0.5f, 0.5f, Vector2(0.0f, 0.0f));
	gameBackground = new Entity(EntityType::BG, 0, 0, 2.5f, 4.0f);

	//MUSIC (I LIKE KIRBY MUSIC)
	music = Mix_LoadMUS("sounds/kirby.mp3");
	Mix_PlayMusic(music, -1);

	SDL_Event event;
	
	bool done = false;
	while (!done) {
		// KEYBOARD INPUTS
		const Uint8 *keys = SDL_GetKeyboardState(NULL);
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				Clean();
				done = true;
			}
			else if (event.type == SDL_KEYDOWN) {
				if (keys[SDL_SCANCODE_LEFT]) {
					player->velocity.x = -2.f;
					player->faceRight = false;
					playerMoving = true;
				}
				else if (keys[SDL_SCANCODE_RIGHT]) {
					player->velocity.x = 2.f;
					player->faceRight = true;
					playerMoving = true;
				}
				else if (keys[SDL_SCANCODE_RETURN]) {
					if (state != 1) {
						if (state == 2) {	//RECYCLE GAME OBJECTS TO REPLAY GAME
							for (Entity * obstacle : obstacles) {
								obstacle->difficulty = 1;
								obstacle->position = Vector2(-3.55f + ((rand() % 20) * 7.1) / 20, 5.f);
								hit = false;
							}
							playerMoving = false;
							player->alive = true;
							player->position = Vector2(0, -1.25f);
							player->faceRight = true;
							score = 0;
							lives = 3;
							Mix_PlayMusic(music, -1);
						}
						state = 1;
					}
				}
			}else if(event.type == SDL_KEYUP){
				if (event.key.keysym.scancode == SDL_SCANCODE_LEFT || event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
					player->velocity = Vector2(0, 0);
					playerMoving = false;
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
