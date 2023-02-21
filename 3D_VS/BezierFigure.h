#pragma once
#include <vector>

#include "shader_handler.h"

class BezierFigure {
public:
	BezierFigure(const std::array<glm::vec3, 16>& pointsBezier, int quality);
	~BezierFigure();

	void draw(const glm::mat4& projection, const glm::mat4& view);

private:
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	unsigned int VAO{}, VBO[2]{}, EBO{};
	const int quality;
	Shader shader;

	void generateBezier(const std::array<glm::vec3, 16>& pointsBezier);
};
