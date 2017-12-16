#include "Entity.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

SpriteSheet::SpriteSheet(unsigned int textureID, float u, float v, float spriteWidth, float spriteHeight, float size) :
	textureID(textureID), u(u), v(v), spriteWidth(spriteWidth), spriteHeight(spriteHeight), size(size) {
};

void SpriteSheet::draw(ShaderProgram * program) {
	glBindTexture(GL_TEXTURE_2D, textureID);
	GLfloat texCoords[] = {
		u, v + spriteHeight,
		u + spriteWidth, v,
		u, v,
		u + spriteWidth, v,
		u, v + spriteHeight,
		u + spriteWidth, v + spriteHeight
	};

	float aspect = spriteWidth / spriteHeight;
	float vertices[] = {
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, 0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, -0.5f * size
	};
	glUseProgram(program->programID);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);

	glEnableVertexAttribArray(program->positionAttribute);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

Entity::Entity(EntityType newType, float x, float y, float width, float height, Vector2 newVel) {
	type = newType;
	velocity.x = newVel.x;
	velocity.y = newVel.y;

	size.x = width;
	size.y = height;

	//DATA INSTANTIATION BY GAME TYPE
	if (type == EntityType::OBSTACLES) {
		Gtexture = LoadTexture("sprites/dirtCenter.png");
		sprite = SheetSprite(Gtexture, 0.f, 0.f, 1.f, 1.f, size);
	}
	else if (type == EntityType::PLAYER) {
		pTexture = LoadTexture("sprites/player.png");
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

	position.x = x;
	position.y = y;

	dimensions.b = position.y - (size.y / 2);
	dimensions.t = position.y + (size.y / 2);
	dimensions.l = position.x - (size.x / 2);
	dimensions.r = position.x + (size.x / 2);
}

void Entity::update(float time) {
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
				sprite = SheetSprite(pTexture, 0.f + int(animationTime) * (53.f / 159.f), 0.f, 53.f / 159.f, 63.f / 126.f, size); //53 x 63, 159 x 126
			}
			else if (!faceRight && alive) {
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

void Entity::draw() {
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

bool Entity::collideWith(Entity& other) {
	if (!(dimensions.b > other.dimensions.t ||
		dimensions.t < other.dimensions.b ||
		dimensions.r < other.dimensions.l ||
		dimensions.l > other.dimensions.r)) {
		return true;
	}
	return false;
}

class Entity {
public:
	Matrix movement;
	Vector4 dimensions;
	Vector2 position, size, velocity;
	GLuint Gtexture, pTexture;
	SheetSprite sprite;
	EntityType type;
	bool faceRight, alive;
	float elapsed, animationTime, invWin;
	int difficulty;
	

	

	

	
};
