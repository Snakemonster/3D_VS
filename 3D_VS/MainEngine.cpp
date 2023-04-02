#include "MainEngine.h"

#include <array>
#include <iostream>
#include <ostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/normal.hpp>

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

	bool pressed = false;
	while (!glfwWindowShouldClose(window)) {
		const auto currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		processInput(window, deltaTime);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		update(deltaTime);

		glfwSwapBuffers(window);
		glfwPollEvents();
		lastTime = currentTime;
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
		pos.x = static_cast<float>(sin(glfwGetTime()) * 8);
		pos.y = static_cast<float>(sin(glfwGetTime() * 2));
		pos.z = static_cast<float>(-cos(glfwGetTime()) * 8);

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

		std::vector<glm::vec3> verticesArray;
		for (int i = 0; i < vertices.size(); i += 3) {
			auto triangle = glm::vec3(vertices[i], vertices[i + 1], vertices[i + 2]);
			verticesArray.push_back(triangle);
		}

		for (int i = 0; i < verticesArray.size(); i += 4) {
			indices.push_back(i);
			indices.push_back(i + 1);
			indices.push_back(i + 2);

			indices.push_back(i + 2);
			indices.push_back(i + 3);
			indices.push_back(i);
		}

		std::vector<float> normals;
		for (int i = 0; i < indices.size(); i += 3) {
			glm::vec3 normal = triangleNormal(verticesArray[indices[i]], verticesArray[indices[i+1]], verticesArray[indices[i+2]]);
			normals.push_back(normal.x);
			normals.push_back(normal.y);
			normals.push_back(normal.z);
			
			normals.push_back(normal.x);
			normals.push_back(normal.y);
			normals.push_back(normal.z);
		}

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
		auto transform = glm::mat4(1.f);
		transform = glm::translate(transform, glm::vec3(3, 0, 0));
		// transform = rotate(transform, 0.5f * (float)glfwGetTime() * glm::radians(45.f), glm::vec3(1.0, 0.0, 0.0));

		auto ambient = glm::vec3(0.752f, 0.607f, 0.227f);
		auto diffuse = glm::vec3(0.247f, 0.1995f, 0.075f);
		auto specular = glm::vec3(0.628f, 0.556f, 0.366f);
		auto n = 0.4f * 128;
		auto Il = 2.5f;

		glm::vec3 Ia_ka = 3.6f * ambient;
		
		shader.use();
		shader.setMat4("model", transform);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
		shader.setMat3("matrixNormals", glm::mat3(glm::transpose(glm::inverse(view * transform))));

		shader.setVec3("material.ambient", ambient);
		shader.setVec3("material.diffuse", diffuse);
		shader.setVec3("material.specular", specular);
		shader.setFloat("material.shininess", n);

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

class Curve {
	Shader shader;
	std::vector<float> vertices;
	int quality;
	unsigned int VAO, VBO;
public:
	std::vector<glm::vec3> Vertices;

	glm::vec3 getLastPos() const {
		return Vertices[quality];
	}

	Curve(int quality) : quality(quality), shader(Shader("vertex_light.glsl", "fragment_line.glsl")) {
		const std::array<glm::vec3, 4> points = {
			glm::vec3(0.f, 0.f, 0.f),
			glm::vec3(-10.f, -6.f, -3.f),
			glm::vec3(8.f, 10.f, 12.f),
			glm::vec3(0.f, 5.f, 20.f),
		};

		auto func = [points](float t) -> glm::vec3 {
			float b0 = pow(1 - t, 3);
			float b1 = 3 * t * pow(1 - t, 2);
			float b2 = 3 * pow(t, 2) * (1 - t);
			float b3 = pow(t, 3);

			// Calculate the interpolated point
			glm::vec3 p = b0 * points[0] + b1 * points[1] + b2 * points[2] + b3 * points[3];

			return p;
		};

		for (int i = 0; i <= this->quality; i++) {
			float t = static_cast<float>(i) / this->quality;
			auto vertex = func(t);
			Vertices.push_back(vertex);
			vertices.push_back(vertex.x);
			vertices.push_back(vertex.y);
			vertices.push_back(vertex.z);
		}

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	~Curve() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}

	void draw(const glm::mat4& projection, const glm::mat4& view) {
		glm::mat4 transform = glm::mat4(1.f);
		
		glLineWidth(3.f);
		shader.use();
		shader.setMat4("model", transform);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, vertices.size() * sizeof(float));
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
bool isMovfingFigure = false;

struct FObj {
	float t;
	int pos;
	Cube* cube;
	LightCube* lightCube;
	BezierFigure* BezierFigure1;
	BezierFigure* BezierFigure2;
	Curve* curve;

	~FObj() {
		delete cube;
		delete lightCube;
		delete BezierFigure1;
		delete BezierFigure2;
		delete curve;
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
	auto obj = new FObj{
		0.f,
		0,
		new Cube(light),
		light,
		new BezierFigure(points1, 100),
		new BezierFigure(points2, 100),
		new Curve(10000),
	};
	return obj;
}

void MainEngine::update(double deltaTime) {
	const glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SRC_WIDTH / (float)SRC_HEIGHT, 0.1f, 100.0f);
	const glm::mat4 view = camera.GetViewMatrix();
	obj->lightCube->draw(projection, view);
	// obj->cube->draw(projection, view);
	obj->BezierFigure1->draw(projection, view, obj->lightCube->getPos());
	obj->BezierFigure2->draw(projection, view, obj->lightCube->getPos());
	obj->curve->draw(projection, view);
	
	if (isMovfingFigure) {
		glm::vec3 pos;
		if(obj->BezierFigure1->getPos() == obj->curve->getLastPos()) {
			pos = glm::vec3(0.f);
			obj->BezierFigure1->setPos(pos);
			obj->BezierFigure2->setPos(pos);
			obj->pos = 0;
			return;
		}
		if (obj->t > 0.002f) {
			obj->pos++;
			obj->BezierFigure1->setPos(obj->curve->Vertices[obj->pos]);
			obj->BezierFigure2->setPos(obj->curve->Vertices[obj->pos]);
			obj->t = 0.f;
		}
		else obj->t += deltaTime;
		
	}
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
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) isMovfingFigure = !isMovfingFigure;
		
	
}
