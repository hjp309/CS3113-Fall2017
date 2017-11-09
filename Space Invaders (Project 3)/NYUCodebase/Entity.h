#ifndef ENTITY
#define ENTITY

#include "ShaderProgram.h"

enum class EntityType { PLAYER, MOB1, MOB2, MOB3, MOB4, MISSILE };

class SpriteSheet {
	float x, y, u, v, aspect, width, height, size;
	unsigned int textureID;
public:
	SpriteSheet() {};
	SpriteSheet(unsigned int textureID, float u, float v, float width, float height);
	void draw(ShaderProgram *program, float * vertices[]);
};

class Entity {
	float height, width, aspect, x, y, u, v, size = 1;
	float * vertices;
	unsigned int textureID;
	SpriteSheet sprite;
	EntityType type;
public:
	Entity(EntityType type, float x, float y);
	//void shoot();
	void update(float time, float velX = 1, float velY = 1);
	void move(float x, float y);
	void draw(ShaderProgram *program);
	bool checkCollision();
};

#endif