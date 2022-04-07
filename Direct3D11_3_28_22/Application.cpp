#include "Application.h"

#include <iostream>

const char* get_glfw_error() {
	const char* error;
	glfwGetError(&error);

	return error;
}

bool Application::is_closing()
{
	return glfwWindowShouldClose(_window);
}

bool Application::init()
{
	// Initalize GLFW
	if (glfwInit() != GLFW_TRUE) {
		std::cout << "Error: GLFW failed to initialize." << std::endl;
		return false;
	}

	// Create a GLFW window that is hidden, unresizeable, and has no client API.
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	_window = glfwCreateWindow(1600, 900, "D3D11 Test", nullptr, nullptr);
	if (_window == nullptr) {
		std::cout << "Error: Failed to create GLFW window. GLFW Error: " << get_glfw_error() << std::endl;
		return false;
	}

	// Set the user point to use with input callbacks
	glfwSetWindowUserPointer(_window, this);

	// Create an instance of the renderer
	_renderer = std::make_unique<Renderer>(_window);

	// Initalize the renderer
	if (!_renderer->init())
		return false;

	glfwShowWindow(_window);

	return true;
}

void Application::run()
{
	glfwPollEvents();

	_renderer->draw();
}
