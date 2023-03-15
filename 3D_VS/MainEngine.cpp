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
class BaseCube {
protected:
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	unsigned int VAO, VBO, EBO;
	Shader shader;

	void initBaseCube() {
		vertices = {
			-0.5f, -0.5f, 0.5f, // bottom-left
			0.5f, -0.5f, 0.5f, // bottom-right
			0.5f, 0.5f, 0.5f, // top-right
			-0.5f, 0.5f, 0.5f, // top-left

			// Back face
			-0.5f, -0.5f, -0.5f, // bottom-left
			0.5f, -0.5f, -0.5f, // bottom-right
			0.5f, 0.5f, -0.5f, // top-right
			-0.5f, 0.5f, -0.5f // top-left
		};

		indices = {
			// Front face
			0, 1, 2,
			2, 3, 0,

			// Top face
			3, 2, 6,
			6, 7, 3,

			// Back face
			7, 6, 5,
			5, 4, 7,

			// Bottom face
			4, 5, 1,
			1, 0, 4,

			// Left face
			0, 3, 7,
			7, 4, 0,

			// Right face
			1, 5, 6,
			6, 2, 1
		};

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
	}

public:
	BaseCube(Shader& shader) : shader(shader) {	}

	virtual ~BaseCube() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VAO);
		glDeleteBuffers(1, &EBO);
		glDeleteBuffers(1, &VBO);
	}

	virtual void draw(const glm::mat4& projection, const glm::mat4& view) = 0;
};


class LightCube : public BaseCube {
protected:
	glm::vec3 pos;

public:
	LightCube(Shader shader = Shader("vertex_light.glsl", "fragment_light.glsl")) : BaseCube(shader) {
		initBaseCube();
		pos = glm::vec3(0.f, 2.f, 0.f);
	}

	glm::vec3 getPos() const {
		return pos;
	}

	virtual void draw(const glm::mat4& projection, const glm::mat4& view) override {
		shader.use();
		pos.x = static_cast<float>(sin(glfwGetTime()) * 4);
		pos.y = static_cast<float>(sin(glfwGetTime() * 2) * cos(glfwGetTime()) * 2);
		pos.z = static_cast<float>(-cos(glfwGetTime()) * 4);

		auto transform = glm::mat4(1.f);
		transform = glm::translate(transform, pos);
		transform = glm::scale(transform, glm::vec3(0.2f));
		shader.setMat4("model", transform);
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size() * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);
	}
};

class Cube : public BaseCube {
protected:
	unsigned int NBO;
	LightCube* lightCube;
public:
	Cube(LightCube* lightCube, Shader shader = Shader("vertex.glsl", "fragment.glsl")) : BaseCube(shader), lightCube(lightCube) {
		vertices = {
			// Front face
			-0.5f, -0.5f,  0.5f,
			 0.5f, -0.5f,  0.5f,
			 0.5f,  0.5f,  0.5f,
			-0.5f,  0.5f,  0.5f,
			// Back face
			-0.5f, -0.5f, -0.5f,
			-0.5f,  0.5f, -0.5f,
			 0.5f,  0.5f, -0.5f,
			 0.5f, -0.5f, -0.5f,
			 // Left face
			-0.5f, -0.5f,  0.5f,
			-0.5f,  0.5f,  0.5f,
			-0.5f,  0.5f, -0.5f,
			-0.5f, -0.5f, -0.5f,
			// Right face
			 0.5f, -0.5f,  0.5f,
			 0.5f, -0.5f, -0.5f,
			 0.5f,  0.5f, -0.5f,
			 0.5f,  0.5f,  0.5f,
			// Top face
			-0.5f,  0.5f,  0.5f,
			 0.5f,  0.5f,  0.5f,			 
			 0.5f,  0.5f, -0.5f,
			-0.5f,  0.5f, -0.5f,
			// Bottom face
			-0.5f, -0.5f,  0.5f,
			-0.5f, -0.5f, -0.5f,
			 0.5f, -0.5f, -0.5f,
			 0.5f, -0.5f,  0.5f,
		};

		indices = {
			0,  1,  2,      2,  3,  0,    // Front face
			4,  5,  6,      6,  7,  4,    // Back face
			8,  9,  10,     10, 11, 8,    // Top face
			12, 13, 14,     14, 15, 12,   // Bottom face
			16, 17, 18,     18, 19, 16,   // Right face
			20, 21, 22,     22, 23, 20,   // Left face
		};
		
		std::vector<float> normals = {
			//front
			0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f,
			//back
			0.0f, 0.0f, -1.0f,
			0.0f, 0.0f, -1.0f,
			0.0f, 0.0f, -1.0f,
			0.0f, 0.0f, -1.0f,
			//left
			-1.0f, 0.0f, 0.0f,
			-1.0f, 0.0f, 0.0f,
			-1.0f, 0.0f, 0.0f,
			-1.0f, 0.0f, 0.0f,
			//right
			1.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			//top
			0.0f, 1.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			//bottom
			0.0f, -1.0f, 0.0f,
			0.0f, -1.0f, 0.0f,
			0.0f, -1.0f, 0.0f,
			0.0f, -1.0f, 0.0f,
		};

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &NBO);
		glBindBuffer(GL_ARRAY_BUFFER, NBO);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
	}

	void draw(const glm::mat4& projection, const glm::mat4& view) override {
		shader.use();

		auto transform = glm::mat4(1.f);
		transform = glm::translate(transform, glm::vec3(3, 0, 0));
		transform = rotate(transform, 0.5f * (float)glfwGetTime() * glm::radians(45.f), glm::vec3(1.0, 0.0, 0.0));

		shader.setMat4("model", transform);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
		shader.setMat3("matrixNormals", glm::mat3(glm::transpose(glm::inverse(view * transform))));

		shader.setVec3("material.ambient", 0.752f, 0.607f, 0.227f);
		shader.setVec3("material.diffuse", 0.247f, 0.1995f, 0.075f);
		shader.setVec3("material.specular", 0.628f, 0.556f, 0.366f);
		shader.setFloat("material.shininess", 0.4f * 128);

		shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
		shader.setVec3("light.ambient", 1.0f, 1.0f, 1.0f);
		shader.setVec3("light.diffuse", 1.0f, 1.0f, 1.0f);
		shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		shader.setVec3("light.position", lightCube->getPos());

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size() * sizeof(unsigned int), GL_UNSIGNED_INT, 0);
	}

	virtual ~Cube() {
		glDeleteBuffers(1, &NBO);
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
	LightCube* lightCube;
	// BezierFigure* BezierFigure1;
	// BezierFigure* BezierFigure2;
	~FObj() {
		delete cube;
		delete lightCube;
		// delete BezierFigure1;
		// delete BezierFigure2;
	}
};

FObj* MainEngine::start() {
	std::array<glm::vec3, 16> points1 = {
		//A
		glm::vec3(0.f, 13.f, 0.f),
		glm::vec3(0.f, 2.f, 0.f),
		glm::vec3(0.f, 12.f, 19.f),
		glm::vec3(0.f, 0.f, 25.f),

		//B
		glm::vec3(4.5f, 7.f, 0.f),
		glm::vec3(2.f, 2.f, 5.f),
		glm::vec3(5.f, 8.f, 18.f),
		glm::vec3(0.f, 0.f, 25.f),

		//C
		glm::vec3(8.75f, 6.f, 0.f),
		glm::vec3(5.f, 1.f, 2.f),
		glm::vec3(8.f, 5.5f, 14.f),
		glm::vec3(0.f, 0.f, 25.f),

		//D
		glm::vec3(11.f, 0.f, 0.f),
		glm::vec3(10.f, 0.f, 10.f),
		glm::vec3(10.f, 0.f, 18.f),
		glm::vec3(0.f, 0.f, 25.f),
	};
	std::array<glm::vec3, 16> points2 = {
		//A
		glm::vec3(0.f, 13.f, 0.f),
		glm::vec3(0.f, 2.f, 0.f),
		glm::vec3(0.f, 12.f, 19.f),
		glm::vec3(0.f, 0.f, 25.f),

		//B
		glm::vec3(-4.5f, 7.f, 0.f),
		glm::vec3(-2.f, 2.f, 5.f),
		glm::vec3(-5.f, 8.f, 18.f),
		glm::vec3(0.f, 0.f, 25.f),

		//C
		glm::vec3(-8.75f, 6.f, 0.f),
		glm::vec3(-5.f, 1.f, 2.f),
		glm::vec3(-8.f, 5.5f, 14.f),
		glm::vec3(0.f, 0.f, 25.f),

		//D
		glm::vec3(-11.f, 0.f, 0.f),
		glm::vec3(-10.f, 0.f, 10.f),
		glm::vec3(-10.f, 0.f, 18.f),
		glm::vec3(0.f, 0.f, 25.f),
	};
	auto light = new LightCube;
	return new FObj{new Cube(light), light,/* new BezierFigure(points1, 100), new BezierFigure(points2, 100)*/};
}

void MainEngine::update() {
	const glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SRC_WIDTH / (float)SRC_HEIGHT, 0.1f,
	                                              100.0f);
	const glm::mat4 view = camera.GetViewMatrix();
	obj->lightCube->draw(projection, view);
	obj->cube->draw(projection, view);
	// obj->BezierFigure1->draw(projection, view);
	// obj->BezierFigure2->draw(projection, view);
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
