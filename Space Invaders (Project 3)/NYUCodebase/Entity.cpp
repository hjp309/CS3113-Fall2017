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

Entity::Entity(float playerX) {
	position.x = playerX;
	position.y = -1.35f;
	velocity = 5.0f;
	timeActive = 0;
	sides = Vector4(position.x - 0.25, position.y + 0.25, position.y - 0.25, position.x + 0.25);
	sprite = SpriteSheet(textureID, 46.0f/104.0f, 16.0f/32.0f, 2.0f/104.0f, 6.0f/32.0f, 0.25f);
}

Entity::Entity(EntityType whatType, float x, float y, float size) : type(whatType){
	position.x = x;
	position.y = y;

	textureID = LoadTexture("invaders.png");
	sides = Vector4(position.x - 0.25, position.y + 0.25, position.y - 0.25, position.x + 0.25);

	switch (type) {
	case EntityType::PLAYER:
		std::cout << "Player spawned." << std::endl;
		velocity = 1.0f;
		sprite = SpriteSheet(textureID, 46.0f/104.0f, 0, 22.0f/104.0f, 16.0f/32.0f, size);
		break;
	case EntityType::MOB:
		std::cout << "Mob spawned." << std::endl;
		velocity = 0.5f;
		sprite = SpriteSheet(textureID, 0, 0, 22.0f/104.0f, 16.0f/32.0f, size);
		break;
	}
}

void Entity::draw(ShaderProgram * program) {
	mvMatrix.Identity();
	mvMatrix.Translate(position.x, position.y, 0);
	program->SetModelviewMatrix(mvMatrix);
	sprite.draw(program);
}

void Entity::update(float elapsed) {
	switch (type) {
	case EntityType::MOB:
		if (position.x + velocity * elapsed > 3.55f) {
			velocity = -abs(velocity);
			position.y -= 0.1f;
		}
		else if (position.x - velocity * elapsed < -3.55f) {
			velocity = abs(velocity);
			position.y -= 0.1f;
		}
		position.x += velocity * elapsed;
		break;
	case EntityType::PLAYER:
		position.x += velocity * elapsed;
		break;
	}
}

Vector2& Entity::getPlayerPos() {
	if (type == EntityType::PLAYER) {
		return position;
	}
	return Vector2(0, 0);
}

bool Entity::collideWith(const Entity &object){
	if (!(sides.b > object.sides.t ||
		sides.t < object.sides.b ||
		sides.r < object.sides.l ||
		sides.l > object.sides.r)) {
		return true;
	}
	return false;
}