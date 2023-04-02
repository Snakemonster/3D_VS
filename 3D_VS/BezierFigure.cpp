#include "BezierFigure.h"

#include <array>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/normal.hpp>
#include <glm/gtx/string_cast.hpp>

#include "stb_image.h"

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

	//textures
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	unsigned char* data = stbi_load("resources/yellow-wall-texture-with-scratches.jpg", &width, &height, &nrChannels, 0);
	if (data) {
		if (nrChannels == 3) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		else if (nrChannels == 4) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	glGenBuffers(1, &TBO);
	glBindBuffer(GL_ARRAY_BUFFER, TBO);
	glBufferData(GL_ARRAY_BUFFER, textures.size() * sizeof(float), textures.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(2);
	//end of textures

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
	glDeleteBuffers(1, &TBO);
}

void BezierFigure::draw(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& lightpos) {
	auto transform = glm::mat4(1.f);
	// transform = glm::translate(transform, glm::vec3(-3.f, 0, 0.f));
	transform = glm::scale(transform, glm::vec3(0.2f));

	auto ambient = glm::vec3(0.752f, 0.607f, 0.227f);
	auto diffuse = glm::vec3(0.247f, 0.1995f, 0.075f);
	auto specular = glm::vec3(0.628f, 0.556f, 0.366f);
	auto n = 128;

	glBindTexture(GL_TEXTURE_2D, texture);

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

	auto texturesMatrix = new glm::vec2*[quality + 1];
	for (int i = 0; i <= quality; i++) texturesMatrix[i] = new glm::vec2[quality + 1];


	for (int u = 0; u <= quality; ++u) {
		for (int v = 0; v <= quality; ++v) {
			auto point = r_uv(TO_FLOAT(u) / quality, TO_FLOAT(v) / quality);
			verticesMatrix[u][v] = point;
			texturesMatrix[u][v] = glm::vec2(TO_FLOAT(u) / quality, TO_FLOAT(v) / quality);
		}
	}

	std::vector<glm::vec3> trueVertices;
	std::vector<glm::vec2> trueTextures;
	for (int i = 0; i < quality; ++i) {
		for (int j = 0; j < quality; ++j) {
			trueVertices.push_back(verticesMatrix[i][j]);			//1
			trueVertices.push_back(verticesMatrix[i][j + 1]);		//2
			trueVertices.push_back(verticesMatrix[i + 1][j]);		//3
			trueVertices.push_back(verticesMatrix[i + 1][j + 1]);	//4

			float coord = 0.5f;
			if (texturesMatrix[i][j].x <= coord && texturesMatrix[i][j].y <= coord)
				trueTextures.push_back(texturesMatrix[i][j]);
			else
				trueTextures.push_back(glm::vec2(0,0));

			if (texturesMatrix[i][j + 1].x <= coord && texturesMatrix[i][j + 1].y <= coord)
				trueTextures.push_back(texturesMatrix[i][j + 1]);
			else
				trueTextures.push_back(glm::vec2(0, 0));

			if (texturesMatrix[i + 1][j].x <= coord && texturesMatrix[i + 1][j].y <= coord)
				trueTextures.push_back(texturesMatrix[i + 1][j]);
			else
				trueTextures.push_back(glm::vec2(0, 0));

			if (texturesMatrix[i + 1][j + 1].x <= coord && texturesMatrix[i + 1][j + 1].y <= coord)
				trueTextures.push_back(texturesMatrix[i + 1][j + 1]);
			else
				trueTextures.push_back(glm::vec2(0, 0));
		}
	}

	for (int i = 0; i <= quality; ++i) delete[] verticesMatrix[i];
	delete[] verticesMatrix;

	for (int i = 0; i <= quality; ++i) delete[] texturesMatrix[i];
	delete[] texturesMatrix;

	for (auto triangle : trueVertices) {
		vertices.push_back(triangle.x);
		vertices.push_back(triangle.y);
		vertices.push_back(triangle.z);
	}

	for(auto texture: trueTextures) {
		textures.push_back(texture.x);
		textures.push_back(texture.y);
	}

	for (int i = 0; i < trueVertices.size(); i += 4) {
		indices.push_back(i);			//1
		indices.push_back(i + 1);	//2
		indices.push_back(i + 2);	//3

		indices.push_back(i + 2);	//3
		indices.push_back(i + 1);	//2
		indices.push_back(i + 3);	//4
	}

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
}
