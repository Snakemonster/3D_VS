#include "BezierFigure.h"

#include <array>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/normal.hpp>
#include <glm/gtx/string_cast.hpp>

#define TO_FLOAT static_cast<float>
/**
 * \brief 
 * \param pointsBezier array of points Bezier, takes only 16 points glm::vec3.
 * \param quality the quality of figure.
 */
BezierFigure::BezierFigure(const std::array<glm::vec3, 16>& pointsBezier, int quality) : quality(quality),
	shader(Shader("vertex.glsl", "fragment.glsl")) {
	generateBezier(pointsBezier);

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

BezierFigure::~BezierFigure() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VAO);
	glDeleteBuffers(1, &EBO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &NBO);
}

void BezierFigure::draw(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& lightpos) {
	auto transform = glm::mat4(1.f);
	// transform = glm::translate(transform, glm::vec3(-3.f, 0, 0.f));
	transform = glm::scale(transform, glm::vec3(0.2f));

	auto ambient = glm::vec3(0.752f, 0.607f, 0.227f);
	auto diffuse = glm::vec3(0.247f, 0.1995f, 0.075f);
	auto specular = glm::vec3(0.628f, 0.556f, 0.366f);
	auto n = 0.4f * 128;

	shader.use();
	shader.setMat4("projection", projection);
	shader.setMat4("view", view);
	shader.setMat4("model", transform);
	shader.setMat3("matrixNormals", glm::mat3(glm::transpose(glm::inverse(view * transform))));
	
	shader.setVec3("material.ambient", ambient);
	shader.setVec3("material.diffuse", diffuse);
	shader.setVec3("material.specular", specular);
	shader.setFloat("material.shininess", n);
	
	shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
	shader.setVec3("light.ambient", 1.0f, 1.0f, 1.0f);
	shader.setVec3("light.diffuse", 1.0f, 1.0f, 1.0f);
	shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

	shader.setFloat("light.constant", 1.0f);
	shader.setFloat("light.linear", 0.09f);
	shader.setFloat("light.quadratic", 0.032f);

	shader.setVec3("light.position", lightpos);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size() * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);
}


void BezierFigure::generateBezier(const std::array<glm::vec3, 16>& pointsBezier) {
	auto r_uv = [&pointsBezier](float u, float v) -> glm::vec3 {
		auto u_vec = glm::vec4(pow(1 - u, 3), 3 * pow(1 - u, 2) * u, 3 * (1 - u) * pow(u, 2), pow(u, 3));
		auto v_vec = glm::vec4(pow(1 - v, 3), 3 * pow(1 - v, 2) * v, 3 * (1 - v) * pow(v, 2), pow(v, 3));

		std::array<glm::vec3, 4> rx{};
		for (int i = 0; i < 4; ++i) rx[i] = pointsBezier[i] * u_vec.x + pointsBezier[i + 4] * u_vec.y +	pointsBezier[i + 8] * u_vec.z + pointsBezier[i + 12] * u_vec.w;

		glm::vec3 res(0.f);
		for (int i = 0; i < 4; ++i) res += rx[i] * v_vec[i];
		return res;
	};

	auto verticesMatrix = new glm::vec3*[quality + 1];
	for (int i = 0; i <= quality; i++) verticesMatrix[i] = new glm::vec3[quality + 1];

	for (int u = 0; u <= quality; ++u) {
		for (int v = 0; v <= quality; ++v) {
			auto point = r_uv(TO_FLOAT(u) / quality, TO_FLOAT(v) / quality);
			verticesMatrix[u][v] = point;
		}
	}
	
	std::vector<glm::vec3> trueVertices;
	for (int i = 0; i < quality; ++i) {
		for (int j = 0; j < quality; ++j) {
			trueVertices.push_back(verticesMatrix[i][j]);			//1
			trueVertices.push_back(verticesMatrix[i][j + 1]);		//2
			trueVertices.push_back(verticesMatrix[i + 1][j]);		//3
			trueVertices.push_back(verticesMatrix[i + 1][j + 1]);	//4
		}
	}

	for (int i = 0; i <= quality; ++i) delete[] verticesMatrix[i];
	delete[] verticesMatrix;

	for (auto triangle : trueVertices) {
		vertices.push_back(triangle.x);
		vertices.push_back(triangle.y);
		vertices.push_back(triangle.z);
	}
	
	for (int i = 0; i < trueVertices.size(); i += 4) {
		indices.push_back(i);			//1
		indices.push_back(i + 1);	//2
		indices.push_back(i + 2);	//3
	
		indices.push_back(i + 2);	//3
		indices.push_back(i + 1);	//2
		indices.push_back(i + 3);	//4
	}

	// std::vector<glm::vec3> tempNormals;
	// tempNormals.assign(vertices.size() / 2 - 1, glm::vec3(0, 0, 0));
	//
	// for (int i = 0; i < indices.size(); i += 3) {
	// 	glm::vec3 normal = glm::triangleNormal(trueVertices[indices[i]], trueVertices[indices[i + 1]], trueVertices[indices[i + 2]]);
	// 	tempNormals[indices[i]] += normal;
	// 	tempNormals[indices[i + 1]] += normal;
	// 	tempNormals[indices[i + 2]] += normal;
	// }
	//
	// for (auto& tempNormal : tempNormals) {
	// 	tempNormal = normalize(tempNormal);
	// }

	for (int i = 0; i < indices.size(); i += 3) {
		glm::vec3 normal = glm::triangleNormal(trueVertices[indices[i]], trueVertices[indices[i + 1]], trueVertices[indices[i + 2]]);
		normal = -normal;
		normals.push_back(normal.x);
		normals.push_back(normal.y);
		normals.push_back(normal.z);

		normals.push_back(normal.x);
		normals.push_back(normal.y);
		normals.push_back(normal.z);
	}

	// std::cout << "trueVertices\t" << trueVertices.size() << std::endl;
	// std::cout << "vertices\t" << vertices.size() << std::endl;
	// std::cout << "indices\t\t" << indices.size() << std::endl;
	// std::cout << "normals\t\t" << normals.size() << std::endl;
	// std::cout << "tempNormals\t" << tempNormals.size() << std::endl;
}