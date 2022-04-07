#pragma once
#include "Renderer.h"

#include <memory>

#include <GLFW/glfw3.h>

class Application
{
private:
	GLFWwindow* _window = nullptr;
	std::unique_ptr<Renderer> _renderer = nullptr;

public:
	// Query functions
	bool is_closing();

	// Initialization and shutdown functions
	bool init();
	bool shutdown();

	void run();
};

