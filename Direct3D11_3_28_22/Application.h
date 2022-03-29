#pragma once
#include "Renderer.h"

#include <memory>

#include <GLFW/glfw3.h>

class Application
{
private:
	std::shared_ptr<GLFWwindow> _window = nullptr;

	Renderer _renderer;

public:
	Application() : _renderer(_window) {}

	bool init();
};

