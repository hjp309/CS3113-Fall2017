#include "Setup.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define SPRITE_COUNT_X 30
#define SPRITE_COUNT_Y 30
int ** bgData;
int ** platformBGdata;

void placeEntity(std::string &type, float x, float y) {
	if (type == "block") {

	}
	else if (type == "checkpoints") {

	}
}

bool readHeader(std::ifstream &stream) {
	std::string line;
	x_tiles = -1;
	y_tiles = -1;
	tile_size = 21;
	while(getline(stream, line)) {
		if (line == "") { break; }
		std::istringstream sStream(line);
		std::string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "width") {
			x_tiles = atoi(value.c_str());
		}
		else if (key == "height") {
			y_tiles = atoi(value.c_str());
		}
	}
	if(x_tiles == -1 || y_tiles == -1) {
		return false;
	}
	else { // allocate our map data
		bgData = new int*[y_tiles];
		platformBGdata = new int*[y_tiles];
		for(int i = 0; i < y_tiles; ++i) {
			bgData[i] = new int [x_tiles];
			platformBGdata[i] = new int [x_tiles];
		}
		return true;
	}
}

bool readLayerData(std::ifstream &stream) {
	std::string line;
	int ** currentLayer = nullptr;
	while (getline(stream, line)) {
		if (line == "") { break; }
		std::istringstream sStream(line);
		std::string key, value;
		getline(sStream, key, '=');

		if (key == "type" && currentLayer == nullptr) {
			if (sStream.str() == key + "=BG") {
				currentLayer = bgData;
			}
			else {
				currentLayer = platformBGdata;
			}
		}

		getline(sStream, value);
		if (key == "data") {
			for (int y = 0; y < y_tiles; y++) {
				getline(stream, line);
				std::istringstream lineStream(line);
				std::string tile;
				for (int x = 0; x < x_tiles; x++) {
					getline(lineStream, tile, ',');
					int val = atoi(tile.c_str());
					if (val > 0) {
						//sprite IDs are indexed from 0 in a flare map txt
						currentLayer[y][x] = val - 1;
					}
					else {
						currentLayer[y][x] = 200;
					}
				}
			}
		}
	}
	return true;
}

bool readEntityData(std::ifstream &stream) {
	std::string line;
	std::string type;
	while (getline(stream, line)) {
		if (line == "") { break; }
		std::istringstream sStream(line);
		std::string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "type") {
			type = value;
		}
		else if (key == "location") {
			std::istringstream lineStream(value);
			std::string xPosition, yPosition;
			getline(lineStream, xPosition, ',');
			getline(lineStream, yPosition, ',');
			float placeX = atoi(xPosition.c_str())*tile_size;
			float placeY = atoi(yPosition.c_str())*-tile_size;
			placeEntity(type, placeX, placeY);
		}
	}
	return true;
}
void readTiles() {
	for (int y = 0; y < y_tiles; y++) {
		for (int x = 0; x < x_tiles; x++) {
			std::cout << y << " : " << x << " " << bgData[y][x] << std::endl;
		}
	}
}

void createTiles() {
	int start_x = -(tile_size * x_tiles) / 2;
	int start_y = (tile_size * y_tiles) / 2;
	for (int y = 0; y < y_tiles; y++) {
		for (int x = 0; x < x_tiles; x++) {
			float u1 = (float)((bgData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
			float v1 = (float)((bgData[y][x]) / SPRITE_COUNT_Y) / (float)SPRITE_COUNT_Y;
			
			float u2 = (float)((platformBGdata[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
			float v2 = (float)((platformBGdata[y][x]) / SPRITE_COUNT_Y) / (float)SPRITE_COUNT_Y;

			float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
			float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;

			bgVertexData.insert(bgVertexData.end(), {
				start_x + (tile_size * x), start_y - (tile_size * y),									//TL, BL, BR
				start_x + (tile_size * x), start_y - (tile_size * y) - tile_size,						//TL, BR, TR
				start_x + (tile_size * x) + tile_size, start_y - (tile_size * y) - tile_size,

				start_x + (tile_size * x), start_y - (tile_size * y),
				start_x + (tile_size * x) + tile_size, start_y - (tile_size * y) - tile_size,
				start_x + (tile_size * x) + tile_size, start_y - (tile_size * y)
			});

			bgTexCoordData.insert(bgTexCoordData.end(), {
				u1, v1,
				u1, v1 + (spriteHeight),
				u1 + spriteWidth, v1 + (spriteHeight),

				u1, v1,
				u1 + spriteWidth, v1 + (spriteHeight),
				u1 + spriteWidth, v1
			});

			platformVertexData.insert(platformVertexData.end(), {
				start_x + (tile_size * x), start_y - (tile_size * y),									//TL, BL, BR
				start_x + (tile_size * x), start_y - (tile_size * y) - tile_size,						//TL, BR, TR
				start_x + (tile_size * x) + tile_size, start_y - (tile_size * y) - tile_size,

				start_x + (tile_size * x), start_y - (tile_size * y),
				start_x + (tile_size * x) + tile_size, start_y - (tile_size * y) - tile_size,
				start_x + (tile_size * x) + tile_size, start_y - (tile_size * y)
			});

			platformTexCoordData.insert(platformTexCoordData.end(), {
				u2, v2,
				u2, v2 + (spriteHeight),
				u2 + spriteWidth, v2 + (spriteHeight),

				u2, v2,
				u2 + spriteWidth, v2 + (spriteHeight),
				u2 + spriteWidth, v2
			});
		}
	}
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

void drawBG(ShaderProgram * program) {
	texturePTR = &LoadTexture("spritesheet_rgba.png");
	for (int i = 0; i < 100; i++) {
		glBindTexture(GL_TEXTURE_2D, *texturePTR);
		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, &bgVertexData[i]);
		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, &bgTexCoordData[i]);
		glEnableVertexAttribArray(program->positionAttribute);
		glEnableVertexAttribArray(program->texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program->positionAttribute);
		glDisableVertexAttribArray(program->texCoordAttribute);
	}
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