#ifndef ENTITY
#define ENTITY

#include "ShaderProgram.h"

class SpriteSheet {
	float x, y, u, v, width, height, size;
	unsigned int textureID;
public:
	SpriteSheet() {};
	SpriteSheet(unsigned int textureID, float x, float y, float u, float v, float width, float height, float size = 1);
	void draw(ShaderProgram *program);
	void move(float x, float y);
};

enum EntityType { PLAYER, MOB1, MOB2, MOB3, MOB4, MISSILE };

class Entity {
	float height, width, x, y, u, v;
	float * vertices;
	unsigned int textureID;
	SpriteSheet sprite;
	EntityType type;
public:
	Entity(EntityType type, float x, float y);
	//void shoot();
	void update(float time, const Uint8 * key);
	void draw(ShaderProgram *program);
};

#endif