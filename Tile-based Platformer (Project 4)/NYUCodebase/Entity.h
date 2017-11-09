#ifndef ENTITY
#define ENTITY
#include "ShaderProgram.h"

GLuint LoadTexture(const char * filePath);

enum EntityType { PLAYER, BLOCK, FLAG, BG };

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
	Vector4(float a, float b, float c, float d): l(a), t(b), b(c), r(d) {}
	float l;
	float t;
	float b;
	float r;
};

class TileSheet {
	float u, v, width, height, tile_size, size;
	GLuint textureID;
public:
	TileSheet() {};
	TileSheet(GLuint textureID, unsigned int spriteID, unsigned int sprite_countx, unsigned int sprite_county, float tile_size, float size = 1);
	void draw(ShaderProgram *program);
};

class Entity {
	Matrix mvMatrix;

	bool player;
	bool dynamic;

	TileSheet * sprite;
	float * vertices;
	GLuint textureID;
public:
	Vector2 velocity;
	Vector3 position;
	Vector4 side;
	float width, height;
	bool faceRight;
	bool collidedTop;
	bool collidedBottom;
	bool collidedLeft;
	bool collidedRight;
	bool consumed;

	Entity(EntityType type, float x, float y, bool dynamic = false);
	void draw(ShaderProgram *program);
	void update(float elapsed);
	bool collideWith(const Entity &object);
};

#endif