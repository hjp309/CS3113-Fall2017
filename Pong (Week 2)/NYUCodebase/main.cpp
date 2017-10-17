#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#define STB_IMAGE_IMPLEMENTATION
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else		
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;
const float projX = 50.0f;
const float projY = 20.0f;

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

class Entity{
	float * vertices;
	bool moveRight = true;
	float elapsed = 0.0f;
	float speedInc = 1.0f;
public:
	float x, y, width, height, xv, yv;
	Entity(float x, float y, float width, float height, float velocity = 5.0f) : x(x), y(y), width(width), height(height), xv(velocity), yv(velocity){
		vertices = new float[8]{ //
			x, y,
			x + width, y,
			x + width, y - height,
			x, y - height
		};
	}
	
	void draw(ShaderProgram *p) {
		glVertexAttribPointer(p->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(p->positionAttribute);
		glDrawArrays(GL_QUADS, 0, 4);
		glDisableVertexAttribArray(p->positionAttribute);
	}
	void update(float time) {
		y += time * yv;

		if (y >= projY)
			y = projY;
		else if (y - height <= -projY)
			y = -projY + height;

		vertices[0] = x;
		vertices[1] = y;
		vertices[2] = x + width;
		vertices[3] = y;
		vertices[4] = x + width;
		vertices[5] = y - height;
		vertices[6] = x;
		vertices[7] = y - height;
	}

	void checkCollision(float &p1x, float &p1y, float &p1w, float &p1h, 
						float &p2x, float &p2y, float &p2w, float &p2h) {
		if (p1x + p1w > x && y - height <= p1y && y >= p1y - p1h)		// LEFT PADDLE
			xv = fabs(xv);
		if (p2x <= x + width && p2y >= y - height && p2y - p2h <= y)	// RIGHT PADDLE
			xv = -fabs(xv);

		if (y >= projY)				//Top
			yv = -fabs(yv);
		if (y - height <= -projY)	//Bottom
			yv = fabs(yv);
	
		if (x + width >= projX || x <= -projX) { //Left or Right goal resets
			xv = -xv;
			x = 0;
			y = 0;
		}
	}
	
	void updateBall(float time) {
		x += time * xv * speedInc;
		y += time * yv * speedInc;

		elapsed += time;
		if (elapsed > 5.0f) { //Increase ball speed every five seconds
			elapsed -= 5.0f;
			speedInc += 0.1f;
		}
		
		vertices[0] = x;
		vertices[1] = y;
		vertices[2] = x + width;
		vertices[3] = y;
		vertices[4] = x + width;
		vertices[5] = y - height;
		vertices[6] = x;
		vertices[7] = y - height;
	}
};

int main(int argc, char *argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 640, 360);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	Matrix projectionMatrix;
	projectionMatrix.SetOrthoProjection(-projX, projX, -projY, projY, -1.0f, 1.0f);
	Matrix modelviewMatrix;

	SDL_Event event;
	bool done = false;
	float lastTicks = 0.0f;

	float margin = 2.0f;
	float racketWidth = 1.5f;
	float racketHeight = 7.5f;
	float racketSpeed = 20.0f;
	float ballSize = 1.5f;
	float ballSpeed = 15.0f;

	Entity playerOne(-projX + margin, racketHeight/2, racketWidth, racketHeight);
	Entity playerTwo(projX - margin - racketWidth, racketHeight/2, racketWidth, racketHeight);
	Entity ball(-ballSize/2, ballSize/2, ballSize, ballSize, ballSpeed);
	while (!done) {
		while (SDL_PollEvent(&event)) {
			const Uint8 *keys = SDL_GetKeyboardState(NULL);
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
				done = true;
			else if (event.type == SDL_KEYDOWN) {
				if (keys[SDL_SCANCODE_LCTRL])
					playerOne.yv = -racketSpeed;
				if (keys[SDL_SCANCODE_LALT])
					playerOne.yv = racketSpeed;
				if (keys[SDL_SCANCODE_LEFT])
					playerTwo.yv = -racketSpeed;
				if (keys[SDL_SCANCODE_RIGHT])
					playerTwo.yv = racketSpeed;
			}
			else {
				playerOne.yv = 0.0f;
				playerTwo.yv = 0.0f;
			}
		}
		glClearColor(1.0, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(program.programID);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		program.SetProjectionMatrix(projectionMatrix);
		program.SetModelviewMatrix(modelviewMatrix);

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastTicks;
		lastTicks = ticks;


		ball.checkCollision(playerOne.x, playerOne.y, playerOne.width, playerOne.height, 
			playerTwo.x, playerTwo.y, playerTwo.width, playerTwo.height);

		playerOne.update(elapsed);
		playerTwo.update(elapsed);
		ball.updateBall(elapsed);

		playerOne.draw(&program);
		playerTwo.draw(&program);
		ball.draw(&program);
		
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}

