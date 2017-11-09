#include "ShaderProgram.h"
#include <vector>

#ifndef SETUP_H
#define SETUP_H

enum class EntityType { PLAYER, MOB1, MOB2, MOB3, MOB4, MISSILE };

class Entity {
	float height, width, aspect, x, y, u, v, size = 1;
	float * vertices;
	unsigned int textureID;
	EntityType type;
public:
	Entity(EntityType type, float x, float y);
	void update(float time, float velX = 1, float velY = 1);
};

extern int x_tiles;
extern int y_tiles;
extern float tile_size;
extern int ** bgData;
extern int ** platformBGdata;
extern unsigned int * texturePTR;
extern std::vector<float> bgVertexData;
extern std::vector<float> bgTexCoordData;
extern std::vector<float> platformVertexData;
extern std::vector<float> platformTexCoordData;

GLuint LoadTexture(const char *filePath);

bool readHeader(std::ifstream &stream);
bool readLayerData(std::ifstream &stream);
bool readEntityData(std::ifstream &stream);
void readTiles();
void createTiles();
void drawBG(ShaderProgram * program);

#endif