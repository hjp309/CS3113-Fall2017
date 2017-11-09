#include "Entity.h"
#define STB_IMAGE_IMPLEMENTATION
#define TILE_SIZE 70
#include "stb_image.h"

/*
SpriteSheet will hold all information of potential sprite.

Entity will put in coordinates and draw that sprite.
*/

GLuint LoadTexture(const char *filePath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);

	if (image == NULL) {
		std::cout << "Unable to load image. Make sure the path is correct\n";
		assert(false);
	}

	GLuint retTexture;
	glGenTextures(1, &retTexture);
	glBindTexture(GL_TEXTURE_2D, retTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(image);
	return retTexture;
}

TileSheet::TileSheet(GLuint textureID, unsigned int spriteID, unsigned int sprite_countx, unsigned int sprite_county, float tile_size, float size) :
	textureID(textureID), tile_size(tile_size), size(size) {
	u = (float)(spriteID % sprite_countx) / (float)sprite_countx;
	v = (float)(spriteID / sprite_county) / (float)sprite_county;

	//std::cout << u << " : " << v <<  " : " << tile_size << std::endl;

	width = 1.0f / (float)sprite_countx;
	height = 1.0f / (float)sprite_county;
};

void TileSheet::draw(ShaderProgram * program) {
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

Entity::Entity(EntityType type, float x, float y, bool dynamic) : dynamic(dynamic) {
	textureID = LoadTexture("spritesheet.png");
	side.t = position.y + height / 2;
	side.b = position.y - height / 2;
	side.l = position.x - width / 2;
	side.r = position.x + width / 2;

	position.x = x;
	position.y = y;
	faceRight = true;
	player = false;
	width = TILE_SIZE;
	height = TILE_SIZE;
	velocity.x = 0;
	velocity.y = 1;

	switch(type){
		case PLAYER:
			sprite = new TileSheet(textureID, 3, 2, 3, TILE_SIZE, 0.5);
			player = true;
			break;
		case BLOCK:
			sprite = new TileSheet(textureID, 0, 2, 3, TILE_SIZE, 0.5);
			break;
		case BG:
			sprite = new TileSheet(textureID, 6, 2, 3, TILE_SIZE, 0.5);
			break;
		case FLAG:
			sprite = new TileSheet(textureID, 1, 2, 3, TILE_SIZE, 0.5);
			consumed = false;
			break;
	}
}

void Entity::draw(ShaderProgram * program) {
	mvMatrix.Identity();
	mvMatrix.Translate(position.x, position.y, 0);
	program->SetModelviewMatrix(mvMatrix);
	sprite->draw(program);
}

void Entity::update(float elapsed) {
	side.t = position.y + height / 2;
	side.b = position.y - height / 2;
	side.l = position.x - width / 2;
	side.r = position.x + width / 2;

	//std::cout << side.l << " " << side.r << std::endl;
	//std::cout << position.x << " " << position.y << std::endl;
	if(player) {
		if (faceRight) {
			velocity.x = fabs(velocity.x);
		}
		else {
			velocity.x = -fabs(velocity.x);
		}
		position.x += velocity.x * elapsed;
		position.y -= velocity.y * elapsed;
	}
}

bool Entity::collideWith(const Entity &object) {
	if (!(side.b > object.side.t || side.t < object.side.b || side.r < object.side.l || side.l > object.side.r)) {
		return true;
	}
	return false;
}