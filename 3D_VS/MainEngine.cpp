#include "MainEngine.h"

#include <iostream>
#include <ostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader_handler.h"
#define GLFW_INCLUDE_NONE

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int MainEngine::launch() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	obj = start();


	while (!glfwWindowShouldClose(window)) {
		processInput(window);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		update();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	clearObj();

	glfwTerminate();
	return 0;
}


//*****************************************************************************************************************
//*****************************************************************************************************************
//Main work is here

class Triangle {
public:
	Triangle(float vertices[], unsigned int VAO, unsigned int VBO, Shader shader) :
		vertices(vertices), VAO(VAO), VBO(VBO), shader(shader) {
	}

	float* vertices;
	unsigned int VAO, VBO;
	Shader shader;
};

struct FObj {
	Triangle* triangle;
};


FObj* MainEngine::start() {
	Shader triangleShader("vertex.glsl", "fragment.glsl");

	float vertices[] = {
		0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f
	};

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	const auto Obj = new FObj;
	Obj->triangle = new Triangle(vertices, VAO, VBO, triangleShader);
	return Obj;
}

void MainEngine::update() {
	obj->triangle->shader.use();
	glBindVertexArray(obj->triangle->VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void MainEngine::clearObj() {
	glDeleteVertexArrays(1, &obj->triangle->VAO);
	glDeleteBuffers(1, &obj->triangle->VAO);

	delete obj->triangle;
	delete obj;
}

//End of main work
//*****************************************************************************************************************
//*****************************************************************************************************************


void MainEngine::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void MainEngine::processInput(GLFWwindow* window) const {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}
