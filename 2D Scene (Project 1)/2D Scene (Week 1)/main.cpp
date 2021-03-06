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
#define RESOURCE_FOLDER "C:\\Users\\hjp30\\OneDrive\\Documents\\Junior_Year\\CS3113\\Week1\\NYUCodebase\\"
#else		
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

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

int main(int argc, char *argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Hw01", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

	glViewport(0, 0, 640, 360);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	Matrix projectionMatrix;
	projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
	Matrix modelviewMatrix;

	SDL_Event event;
	bool done = false;

	GLuint grassTexture = LoadTexture("images/grass.jpg");
	GLuint mobTexture = LoadTexture("images/mob.png");
	GLuint detTexture = LoadTexture("images/detective.png");

	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		glClearColor(0.4f, 0.2f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(program.programID);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		program.SetModelviewMatrix(modelviewMatrix);
		program.SetProjectionMatrix(projectionMatrix);

		glBindTexture(GL_TEXTURE_2D, grassTexture);
		float grassVertices[] = {
			-3.55f, -2.0f,
			-3.55f, 2.0f,
			3.55f, 2.0f,
			-3.55f, -2.0f,
			3.55f, -2.0f,
			3.55f, 2.0f
		};
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, grassVertices);
		glEnableVertexAttribArray(program.positionAttribute);

		float grassTextureCoor[] = {
			0.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 1.0f,
			0.0f, 0.0f,
			1.0f, 0.0f,
			1.0f, 1.0f
		};
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, grassTextureCoor);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		glBindTexture(GL_TEXTURE_2D, mobTexture);
		float mobVertices[] = {
			3.0f, 2.0f,
			3.0f, -1.0f,
			0.0f, -1.0f,
			3.0f, 2.0f,
			0.0f, 2.0f,
			0.0f, -1.0f
			
		};
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, mobVertices);
		glEnableVertexAttribArray(program.positionAttribute);

		float mobTextureCoor[] = {
			1.0f, 1.0f,
			1.0f, 0.0f,
			0.0f, 0.0f,
			1.0f, 1.0f,
			0.0f, 1.0f,
			0.0f, 0.0f
		};
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, mobTextureCoor);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		glBindTexture(GL_TEXTURE_2D, detTexture);
		float detVertices[] = {
			0.0f, 2.0f,
			-2.0f, 2.0f,
			-2.0f, 0.0f,
			-2.0f, 0.0f,
			0.0f, 0.0f,
			0.0f, 2.0f
		};
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, detVertices);
		glEnableVertexAttribArray(program.positionAttribute);

		float detTextureCoor[] = {
			0.0f, 1.0f,
			-1.0f, 1.0f,
			-1.0f, 0.0f,
			-1.0f, 0.0f,
			0.0f, 0.0f,
			0.0f, 1.0f
		};
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, detTextureCoor);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
