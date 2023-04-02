#pragma once
#include <vector>

#include "shader_handler.h"

class BezierFigure {
public:
	BezierFigure(const std::array<glm::vec3, 16>& pointsBezier, int quality);
	~BezierFigure();

	void draw(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& lightpos);

private:
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	std::vector<float> normals;
	std::vector<float> textures;
	unsigned int VAO{}, VBO{}, EBO{}, NBO{}, TBO{};
	unsigned int texture;
	const int quality;
	Shader shader;

	void generateBezier(const std::array<glm::vec3, 16>& pointsBezier);
};
