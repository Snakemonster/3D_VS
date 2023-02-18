#pragma once
#ifndef MAINENGINE_H
#define MAINENGINE_H

class GLFWwindow;
struct FObj;

class MainEngine {
public:
	int launch();

private:
	FObj* obj;
	FObj* start();
	void update();
	void clearObj();


	GLFWwindow* window;
	static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
	void processInput(GLFWwindow* window) const;
};
#endif
