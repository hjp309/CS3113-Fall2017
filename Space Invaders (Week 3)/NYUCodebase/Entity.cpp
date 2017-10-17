#include "Entity.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

SpriteSheet::SpriteSheet(unsigned int textureID, float x, float y, float u, float v, float width, float height, float size) :
	textureID(textureID), x(x), y(y), u(u), v(v), width(width), height(height), size(size) {};

void SpriteSheet::draw(ShaderProgram * program) {
	glBindTexture(GL_TEXTURE_2D, textureID);	
	GLfloat texCoords[] = {
		u, v + height,
		u + width, v,
		u, v,
		u + width, v,
		u, v + height,
		u + width, v + height
	};

	float aspect = width / height;
	float vertices[] = {
		(x - 0.5f) * size * aspect, (y - 0.5) * size,
		(x + 0.5f) * size * aspect, (y + 0.5) * size,
		(x - 0.5f) * size * aspect, (y + 0.5) * size,
		(x + 0.5f) * size * aspect, (y + 0.5) * size,
		(x - 0.5f) * size * aspect, (y - 0.5) * size,
		(x + 0.5f) * size * aspect, (y - 0.5) * size,
	};

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->positionAttribute);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

void SpriteSheet::move(float x, float y) {
	x += x;
	y += y;
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

Entity::Entity(EntityType whatType, float x, float y) : type(whatType), x(x), y(y) {
	switch (type) {
	case PLAYER:
		std::cout << "Player" << std::endl;
		u = 46;
		v = 0;
		width = 22;
		height = 16;
		break;
	case MOB1:
		std::cout << "Mob 1" << std::endl;
		u = 0;
		v = 0;
		width = 22;
		height = 16;
		break;
	case MOB2:
	case MOB3: 
	case MOB4: 
	case MISSILE:
		;
	}

	textureID = LoadTexture("invaders.png");
	sprite = SpriteSheet(textureID, x, y, u / 104.f, v / 32.f, width / 104.0f, height / 32.0f);
	
}

void Entity::update(float time, const Uint8 * key) {
	switch (type) {
	case PLAYER:
		sprite.move( * time, 0);
	}
}

void Entity::draw(ShaderProgram * program) {
	sprite.draw(program);
}