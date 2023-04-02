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
	void update(double deltaTime);
	void clearObj();

	static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
	static void mouseCallBack(GLFWwindow* windows, double xpos, double ypos);
	void processInput(GLFWwindow* window, double deltaTime) const;
};
#endif
