#pragma once

#include <memory>
#include <iostream>
#include <vector>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <dxgi1_6.h>
#include <d3d11_4.h>
//#include <DirectXTex.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include <SimpleMath.h>

typedef std::vector<byte> ShaderByteCode;

struct StaticVertices {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 texcord;
};

struct ColorBuffer {
	ComPtr<ID3D11Texture2D> buffer = nullptr;
	ComPtr<ID3D11RenderTargetView> RTV = nullptr;
	ComPtr<ID3D11ShaderResourceView> SRV = nullptr;
};

struct DepthBuffer {
	ComPtr<ID3D11Texture2D> buffer = nullptr;
	ComPtr<ID3D11DepthStencilView> DSV = nullptr;
	ComPtr<ID3D11ShaderResourceView> SRV = nullptr;
};

class Renderer
{
private:
	// Renderer resources
	struct WindowSize {
		int32_t width = 0;
		int32_t height = 0;
	} _windowSize;

	// GLFW resources
	GLFWwindow* _window = nullptr;

	// DXGI resources
	std::vector<ComPtr<IDXGIAdapter4>> _avaliableAdapters;
	ComPtr<IDXGIAdapter4> _adapter = nullptr;
	ComPtr<IDXGIFactory7> _factory = nullptr;
	struct Swapchain {
		ColorBuffer backBuffer;
		ComPtr<IDXGISwapChain4> swapchain = nullptr;
		DXGI_FORMAT format;
	} _swapchain;

	// D3D11 resources
#ifndef DEBUG
	ComPtr<ID3D11Debug> _debug = nullptr;
	ComPtr<ID3D11InfoQueue> _debugQueue = nullptr;
#endif
	ComPtr<ID3D11Device5> _device = nullptr;
	ComPtr<ID3D11DeviceContext4> _context = nullptr;
	ComPtr<ID3D11VertexShader> _vertexShader = nullptr;
	ComPtr<ID3D11PixelShader> _pixelShader = nullptr;
	ComPtr<ID3D11DepthStencilState> _depthStencilState = nullptr;
	struct GBuffer {
		DepthBuffer depthBuffer;
		ColorBuffer vertNormalUVCord;
		ColorBuffer materialID;
	} _gBuffer;

	// Helper functions
	ShaderByteCode load_compiled_shader(const char* shaderPath);
	
	// Initalization functions
	bool init_direct3D11();
	bool init_swapchain();
	// Initalizes the render target views and shader resource views for deferred rendering.
	bool init_g_buffer();
	bool init_shaders();
	bool init_input_layouts();

public:
	Renderer(GLFWwindow* window) : _window(window) {}

	bool init();
	void shutdown();

	void draw();
};

