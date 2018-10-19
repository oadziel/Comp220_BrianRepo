#include <iostream>
#include <SDL.h>
#include <GL\glew.h>
#include <SDL_opengl.h>

#include "Shaders.h"

#include <glm\glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

int IsFullScreen(SDL_Surface *surface)
{
	// Return true if fullscreen
	if (surface->flags & SDL_WINDOW_FULLSCREEN) return 1;

	// Return false if windowed
	return 0;
}

int main(int argc, char ** argsv)
{
	
	//Initialises the SDL Library, passing in SDL_INIT_VIDEO to only initialise the video subsystems
	//https://wiki.libsdl.org/SDL_Init
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		//Display an error message box
		//https://wiki.libsdl.org/SDL_ShowSimpleMessageBox
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL_Init failed", SDL_GetError(), NULL);
		return 1;
	}

	//Create a window, note we have to free the pointer returned using the DestroyWindow Function
	//https://wiki.libsdl.org/SDL_CreateWindow
	//Create SDL Window size 1600 x 900 resizable
	SDL_Window* window = SDL_CreateWindow("SDL2 Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1600, 900, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	//Checks to see if the window has been created, the pointer will have a value of some kind
	if (window == nullptr)
	{
		//Show error
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL_CreateWindow failed", SDL_GetError(), NULL);
		//Close the SDL Library
		//https://wiki.libsdl.org/SDL_Quit
		SDL_Quit();
		return 1;
	}

	//Request 3.2 Core OpenGL
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GLContext gl_Context = SDL_GL_CreateContext(window);
	if (gl_Context == nullptr)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL_CreateContext Failed", SDL_GetError(), NULL);

		SDL_DestroyWindow(window);
		SDL_Quit();

		return 1;
	}

	//init GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		//Show error
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "GLEW Initialisation Failed", (char*)glewGetErrorString(err), NULL);
		SDL_DestroyWindow(window);
		SDL_Quit();

		return 1;
	}

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// An array of 3 vectors which represents 3 vertices
	static const GLfloat g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f,  1.0f, 0.0f,
	};

	// This will identify our vertex buffer
	GLuint vertexbuffer;
	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);
	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	//Projection Matrix 45 Degree FOV 16:9 Aspect Ratio display range of 0.1 to 100
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)16.0f / (float)9.0f, 0.1f, 100.0f);

	//camera Matrix
	glm::mat4 View = glm::lookAt(
		glm::vec3(4, 3, 3), //camera is at (4,3,3) in world space
		glm::vec3(0, 0, 0), //Looks at Origin
		glm::vec3(0, 1, 0) //Up is positive Y-Axis
	);
	
	//Model Matrix : Identity Matrix (Model at origin)
	glm::mat4 Model = glm::translate(glm::vec3(0.0f, 0.5f, 0.0f));
	Model = glm::scale(Model, glm::vec3(1.2f, 5.5f, 1.0f));
	float Angle = glm::pi<float>() * 0.5f;
	glm::vec3 Axis(1, 0, 0);
	Model = glm::rotate(Model, Angle, Axis);
	



	//MVP (ModelViewProjection)
	glm::mat4 mvp = Projection*View *Model;

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("vert.glsl", "frag.glsl");

	//Get Handle for "MVP"
	//Only during init
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");



	//Event loop, we will loop until running is set to false, usually if escape has been pressed or window is closed
	bool running = true;
	//SDL Event structure, this will be checked in the while loop
	SDL_Event ev;
	SDL_Surface *mwindow;
	while (running)
	{
		
		//Poll for the events which have happened in this frame
		//https://wiki.libsdl.org/SDL_PollEvent
		while (SDL_PollEvent(&ev))
		{
			//Switch case for every message we are intereted in
			switch (ev.type)
			{
				//QUIT Message, usually called when the window has been closed
			case SDL_QUIT:
				running = false;
				break;
				//KEYDOWN Message, called when a key has been pressed down
			case SDL_KEYDOWN:
				//Check the actual key code of the key that has been pressed
				switch (ev.key.keysym.sym)
				{
					//Escape key
				case SDLK_ESCAPE:
					running = false;
					break;
				//Set Window Fullscreen on press 1 on keyboard
				case SDLK_1:
					SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
					break;
				}
			}
		}
		//Do rendering here!
		glClearColor(0.0, 10.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(programID);

		//Send transform to currently bound Shader
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
		glDisableVertexAttribArray(0);

		SDL_GL_SwapWindow(window);
	}

	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteProgram(programID);
	//Delete Context
	SDL_GL_DeleteContext(gl_Context);
	//Destroy the window and quit SDL2, NB we should do this after all cleanup in this order!!!
	//https://wiki.libsdl.org/SDL_DestroyWindow
	SDL_DestroyWindow(window);
	//https://wiki.libsdl.org/SDL_Quit
	SDL_Quit();

	return 0;
}
