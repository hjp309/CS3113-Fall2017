#ifndef ENTITY
#define ENTITY
#include "ShaderProgram.h"
#include <vector>

GLuint LoadTexture(const char * filePath);

enum EntityType { PLAYER, MOB, MISSILE };

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

class SpriteSheet {
	float u, v, spriteWidth, spriteHeight, size;
	GLuint textureID;
public:
	SpriteSheet() {};
	SpriteSheet(unsigned int textureID, float u, float v, float spriteWidth, float spriteHeight, float size = 1);
	void draw(ShaderProgram *program);
};

class Entity {
	EntityType type;
	float size;
	GLuint textureID;
	SpriteSheet sprite;
	Matrix mvMatrix;
public:
	Vector2 position;
	Vector4 sides;
	float velocity, timeActive;
	Entity(float playerX);
	Entity(EntityType whatType, float x, float y, float size = 1);
	void draw(ShaderProgram *program);
	void update(float elapsed);
	Vector2& getPlayerPos();
	bool collideWith(const Entity &object);
};
#endif