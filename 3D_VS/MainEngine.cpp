#include "MainEngine.h"

#include <array>
#include <iostream>
#include <ostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>

#include "BezierFigure.h"
#include "camera.h"
#include "shader_handler.h"
#define GLFW_INCLUDE_NONE

const unsigned int SRC_WIDTH = 1280;
const unsigned int SRC_HEIGHT = 800;


int MainEngine::launch() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	auto window = glfwCreateWindow(SRC_WIDTH, SRC_HEIGHT, "OpenGL", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetCursorPosCallback(window, mouseCallBack);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	double deltaTime = 0, lastTime = 0;

	obj = start();

	while (!glfwWindowShouldClose(window)) {
		const auto currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		processInput(window, deltaTime);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		update();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	clearObj();

	glfwTerminate();
	return 0;
}

//Additional classes **********************************************************************************************
class Cube {
private:
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	unsigned int VAO, EBO;
	unsigned int VBO[2];
	Shader shader;
public:
	Cube() : shader(Shader("vertex.glsl", "fragment.glsl")) {
		std::vector<glm::vec3> colors = {
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),

		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),

		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		};

		vertices = {
			-0.5f, -0.5f, -0.5f, // vertex 0
			-0.5f, -0.5f, 0.5f, // vertex 1
			-0.5f, 0.5f, -0.5f, // vertex 2
			-0.5f, 0.5f, 0.5f, // vertex 3
			0.5f, -0.5f, -0.5f, // vertex 4
			0.5f, -0.5f, 0.5f, // vertex 5
			0.5f, 0.5f, -0.5f, // vertex 6
			0.5f, 0.5f, 0.5f // vertex 7
		};

		indices = {
			0, 1, 2, // front
			1, 3, 2,
			4, 0, 6, // back
			6, 0, 2,
			5, 4, 7, // right
			4, 6, 7,
			1, 5, 3, // left
			5, 7, 3,
			2, 3, 6, // top
			3, 7, 6,
			1, 0, 5, // bottom
			0, 4, 5
		};

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &VBO[1]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
		glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);

		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
	}

	void draw(const glm::mat4& projection, const glm::mat4& view) {
		auto transform = glm::mat4(1.f);
		transform = glm::translate(transform, glm::vec3(3, 0, 0));
		transform = rotate(transform, (float)glfwGetTime() * glm::radians(45.f), glm::vec3(0.5, 0, 1.));
		shader.use();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		shader.setMat4("transform", transform);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size() * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);
	}
	
	~Cube() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VAO);
		glDeleteBuffers(1, &EBO);
		glDeleteBuffers(1, &VBO[0]);
		glDeleteBuffers(1, &VBO[1]);
	}
};
//Additional classes **********************************************************************************************


//*****************************************************************************************************************
//*****************************************************************************************************************
//Main work is here

//Camera settings
Camera camera = Camera(glm::vec3(0.f, 1.f, 10.f));
bool firstMouse = true;
float lastX = SRC_WIDTH / 2.f, lastY = SRC_HEIGHT / 2.f;


struct FObj {

	Cube* cube;
	BezierFigure* BezierFigure1;
	BezierFigure* BezierFigure2;
	~FObj() {
		delete cube;
		delete BezierFigure1;
		delete BezierFigure2;
	}
};

FObj* MainEngine::start() {
	std::array<glm::vec3, 16> points1 = {
		//A
		glm::vec3(0.f,13.f,0.f),
		glm::vec3(0.f,2.f,0.f),
		glm::vec3(0.f,12.f, 19.f),
		glm::vec3(0.f,0.f,25.f),
		
		//B
		glm::vec3(4.5f, 7.f, 0.f),
		glm::vec3(2.f, 2.f, 5.f),
		glm::vec3(5.f, 8.f, 18.f),
		glm::vec3(0.f,0.f,25.f),
		
		//C
		glm::vec3(8.75f, 6.f, 0.f),
		glm::vec3(5.f, 1.f, 2.f),
		glm::vec3(8.f, 5.5f, 14.f),
		glm::vec3(0.f,0.f,25.f),
		
		//D
		glm::vec3(11.f,0.f, 0.f),
		glm::vec3(10.f, 0.f, 10.f),
		glm::vec3(10.f, 0.f, 18.f),
		glm::vec3(0.f,0.f,25.f),
	};
	std::array<glm::vec3, 16> points2 = {
		//A
		glm::vec3(0.f,13.f,0.f),
		glm::vec3(0.f,2.f,0.f),
		glm::vec3(0.f,12.f, 19.f),
		glm::vec3(0.f,0.f,25.f),

		//B
		glm::vec3(-4.5f, 7.f, 0.f),
		glm::vec3(-2.f, 2.f, 5.f),
		glm::vec3(-5.f, 8.f, 18.f),
		glm::vec3(0.f,0.f,25.f),

		//C
		glm::vec3(-8.75f, 6.f, 0.f),
		glm::vec3(-5.f, 1.f, 2.f),
		glm::vec3(-8.f, 5.5f, 14.f),
		glm::vec3(0.f,0.f,25.f),

		//D
		glm::vec3(-11.f,0.f, 0.f),
		glm::vec3(-10.f, 0.f, 10.f),
		glm::vec3(-10.f, 0.f, 18.f),
		glm::vec3(0.f,0.f,25.f),
	};
	
	const auto Obj = new FObj{new Cube, new BezierFigure(points1, 100), new BezierFigure(points2, 100)};
	return Obj;
}

void MainEngine::update() {
	const glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SRC_WIDTH / (float)SRC_HEIGHT, 0.1f, 100.0f);
	const glm::mat4 view = camera.GetViewMatrix();
	obj->cube->draw(projection, view);
	obj->BezierFigure1->draw(projection, view);
	obj->BezierFigure2->draw(projection, view);
}

void MainEngine::clearObj() {
	delete obj;
}

//End of main work
//*****************************************************************************************************************
//*****************************************************************************************************************

//callbacks
void MainEngine::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void MainEngine::mouseCallBack(GLFWwindow* windows, double xposIn, double yposIn) {
	auto xpos = static_cast<float>(xposIn);
	auto ypos = static_cast<float>(yposIn);

	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}
//end of callbacks

void MainEngine::processInput(GLFWwindow* window, double deltaTime) const {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);
}
