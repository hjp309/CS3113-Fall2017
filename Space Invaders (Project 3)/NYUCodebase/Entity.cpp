#include "Entity.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

SpriteSheet::SpriteSheet(unsigned int textureID, float u, float v, float width, float height) :
	textureID(textureID), u(u), v(v), width(width), height(height), size(size) {
};

void SpriteSheet::draw(ShaderProgram * program, float * vertices[]) {
	glBindTexture(GL_TEXTURE_2D, textureID);	
	GLfloat texCoords[] = {
		u, v + height,
		u + width, v,
		u, v,
		u + width, v,
		u, v + height,
		u + width, v + height
	};

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, *vertices);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->positionAttribute);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
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



Entity::Entity(EntityType whatType, float x, float y) : type(whatType), x(x), y(y) {
	switch (whatType) {
	case EntityType::PLAYER:
		std::cout << "Player" << std::endl;
		u = 46;
		v = 0;
		width = 22;
		height = 16;
		break;
	case EntityType::MOB1:
		std::cout << "Mob 1" << std::endl;
		u = 0;
		v = 0;
		width = 22;
		height = 16;
		break;
	case EntityType::MOB2:
		std::cout << "Mob 2" << std::endl;
		u = 22;
		v = 0;
		width = 22;
		height = 16;
		break;
	case EntityType::MOB3:
		std::cout << "Mob 3" << std::endl;
		u = 0;
		v = 16;
		width = 22;
		height = 16;
		break;
	case EntityType::MOB4:
		std::cout << "Mob 4" << std::endl;
		u = 22;
		v = 16;
		width = 22;
		height = 16;
		break;
	case EntityType::MISSILE:
		break;
	}

	size = 1;
	aspect = width / height;
	vertices = new float[12];
	vertices[0] = (x - 0.5f) * size * aspect;
	vertices[1] = (y - 0.5f) * size;
	vertices[2] = (x + 0.5f) * size * aspect;
	vertices[3] = (y + 0.5) * size;
	vertices[4] = (x - 0.5f) * size * aspect;
	vertices[5] = (y + 0.5) * size;
	vertices[6] = (x + 0.5f) * size * aspect;
	vertices[7] = (y + 0.5) * size;
	vertices[8] = (x - 0.5f) * size * aspect;
	vertices[9] = (y - 0.5) * size;
	vertices[10] = (x + 0.5f) * size * aspect;
	vertices[11] = (y - 0.5) * size;

	textureID = LoadTexture("invaders.png");
	sprite = SpriteSheet(textureID, u / 104.f, v / 32.f, width / 104.0f, height / 32.0f);	
}

void Entity::update(float time, float velX, float velY) {	
	switch (type) {
	case EntityType::PLAYER:
		x += velX;
		y += velY;
		if (x > 1.5f)
			x == 2.0f;
		break;
	case EntityType::MOB1:
		
		break;
	case EntityType::MOB2:
		break;
	case EntityType::MOB3:
		break;
	case EntityType::MOB4:
		break;
	}

	vertices[0] = (x - 0.5f) * size * aspect;
	vertices[1] = (y - 0.5f) * size;
	vertices[2] = (x + 0.5f) * size * aspect;
	vertices[3] = (y + 0.5) * size;
	vertices[4] = (x - 0.5f) * size * aspect;
	vertices[5] = (y + 0.5) * size;
	vertices[6] = (x + 0.5f) * size * aspect;
	vertices[7] = (y + 0.5) * size;
	vertices[8] = (x - 0.5f) * size * aspect;
	vertices[9] = (y - 0.5) * size;
	vertices[10] = (x + 0.5f) * size * aspect;
	vertices[11] = (y - 0.5) * size;
}

void Entity::move(float x, float y) {
}

void Entity::draw(ShaderProgram * program) {
	sprite.draw(program, &vertices);
}

bool Entity::checkCollision() {
	return true;
}