#pragma once

#include <memory>
#include <iostream>
#include <vector>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <dxgi.h>
#include <d3d11.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

class Renderer
{
private:
	// Renderer resources
	struct WindowSize {
		int32_t width = 0;
		int32_t height = 0;
	} _windowSize;

	// GLFW resources
	std::shared_ptr<GLFWwindow> _window = nullptr;

	// DXGI resources
	ComPtr<IDXGIFactory> _factory = nullptr;
	ComPtr<IDXGIAdapter> _adapter = nullptr;
	ComPtr<IDXGISwapChain> _swapchain = nullptr;

	// D3D11 resources
#ifndef DEBUG
	ComPtr<ID3D11Debug> _debug = nullptr;
	ComPtr<ID3D11InfoQueue> _debugQueue = nullptr;
#endif
	ComPtr<ID3D11Device> _device = nullptr;
	ComPtr<ID3D11DeviceContext> _context = nullptr;
	
	// Initalization functions
	bool init_dxgi_factory();
	bool init_adapter();
	bool init_device_and_context();
	bool init_swapchain();

public:
	Renderer(std::shared_ptr<GLFWwindow> window) : _window(window) {}

	bool init();
};

