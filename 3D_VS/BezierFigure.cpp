#include "BezierFigure.h"

#include <array>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#define TO_FLOAT static_cast<float>
/**
 * \brief 
 * \param pointsBezier array of points Bezier, takes only 16 points glm::vec3.
 * \param quality the quality of figure.
 */
BezierFigure::BezierFigure(const std::array<glm::vec3, 16>& pointsBezier, int quality) : quality(quality),
	shader(Shader("vertex.glsl", "fragment.glsl")) {
	generateBezier(pointsBezier);

	std::vector<glm::vec3> colors;
	for (auto i = 0; i < vertices.size() /3 / 4; ++i) {
		for (int j = 0; j < vertices.size() / 3 / quality / 4; ++j) colors.emplace_back(0.1f, 0.2f, 1.f);
		for (int j = 0; j < vertices.size() / 3 / quality / 4; ++j) colors.emplace_back(0.5f, 0.5f, 1.f);
		for (int j = 0; j < vertices.size() / 3 / quality / 4; ++j) colors.emplace_back(0.f, 0.5f, 0.5f);
		for (int j = 0; j < vertices.size() / 3 / quality / 4; ++j) colors.emplace_back(0.5f, 1.f, 0.5f);
	}

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

BezierFigure::~BezierFigure() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VAO);
	glDeleteBuffers(1, &EBO);
	glDeleteBuffers(1, &VBO[0]);
	glDeleteBuffers(1, &VBO[1]);
}

void BezierFigure::draw(const glm::mat4& projection, const glm::mat4& view) {
	auto transform = glm::mat4(1.f);
	transform = glm::translate(transform, glm::vec3(-3.f, 0, 0.f));
	transform = glm::scale(transform, glm::vec3(0.2f));
	shader.use();
	shader.setMat4("projection", projection);
	shader.setMat4("view", view);
	shader.setMat4("transform", transform);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size() * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);
}


void BezierFigure::generateBezier(const std::array<glm::vec3, 16>& pointsBezier) {

	auto test_ruv = [&pointsBezier](float u, float v) -> glm::vec3 {

		auto u_vec = glm::vec4(pow(1 - u, 3), 3 * pow(1 - u, 2) * u, 3 * (1 - u) * pow(u, 2), pow(u, 3));
		auto v_vec = glm::vec4(pow(1 - v, 3), 3 * pow(1 - v, 2) * v, 3 * (1 - v) * pow(v, 2), pow(v, 3));

		std::array<glm::vec3, 4> rx{};
		for (int i = 0; i < 4; ++i) {
			rx[i] = pointsBezier[i] * u_vec.x + pointsBezier[i + 4] * u_vec.y + pointsBezier[i + 8] * u_vec.z + pointsBezier[i + 12] * u_vec.w;
		}

		glm::vec3 res(0.f);
		for (int i = 0; i < 4; ++i) res += rx[i] * v_vec[i]; 
		return res;
	};

	for (int u = 0; u <= quality; ++u) {
		for (int v = 0; v <= quality; ++v) {
			auto point = test_ruv( TO_FLOAT(u)/ quality, TO_FLOAT(v) / quality);
			vertices.push_back(point.x);
			vertices.push_back(point.y);
			vertices.push_back(point.z);
		}
	}
	int points_count = vertices.size() / 3 / (quality + 1), points_all = vertices.size() / 3;
	for (int i = 0; i < points_all - points_count; i += points_count) {
		for (int j = 0; j < points_count - 1; ++j) {
			indices.push_back(i + j);
			indices.push_back(i + j + 1);
			indices.push_back(i + j + quality + 1);
			
			indices.push_back(i + j + 1);
			indices.push_back(i + j + 1 + quality + 1);
			indices.push_back(i + j + quality + 1);
		}
	}
}
