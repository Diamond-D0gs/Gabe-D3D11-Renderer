#include "Renderer.h"

#include <Winuser.h>
#include <cstdio>

namespace Colors {
    XMGLOBALCONST DirectX::XMFLOAT4 White{ 1.0f, 1.0f, 1.0f, 1.0f };
    XMGLOBALCONST DirectX::XMFLOAT4 Black{ 0.0f, 0.0f, 0.0f, 1.0f };
    XMGLOBALCONST DirectX::XMFLOAT4 Red { 1.0f, 0.0f, 0.0f, 1.0f };
    XMGLOBALCONST DirectX::XMFLOAT4 Green { 0.0f, 1.0f, 0.0f, 1.0f };
    XMGLOBALCONST DirectX::XMFLOAT4 Blue { 0.0f, 0.0f, 1.0f, 1.0f };
    XMGLOBALCONST DirectX::XMFLOAT4 Yellow { 1.0f, 1.0f, 0.0f, 1.0f };
    XMGLOBALCONST DirectX::XMFLOAT4 Cyan { 0.0f, 1.0f, 1.0f, 1.0f };
    XMGLOBALCONST DirectX::XMFLOAT4 Magenta { 1.0f, 0.0f, 1.0f, 1.0f };
}

void Renderer::shutdown()
{
    return;
}

ShaderByteCode Renderer::load_compiled_shader(const char* shaderPath)
{
    ShaderByteCode shaderByteCode;

    FILE* shaderFile = nullptr;
    fopen_s(&shaderFile, shaderPath, "rb");
    if (shaderFile == nullptr) {
        std::cout << "Shader Loader Error: Failed to open file \"" << shaderPath << "\".\n";
        return shaderByteCode;
    }

    fseek(shaderFile, 0, SEEK_END);
    size_t shaderSize = ftell(shaderFile);
    rewind(shaderFile);
    if (shaderSize <= 0) {
        std::cout << "Shader Loader Error: File \"" << shaderPath << "\" has no size.\n";
        fclose(shaderFile);
        return shaderByteCode;
    }

    shaderByteCode.resize(shaderSize);
    size_t bytesWritten = fread_s(shaderByteCode.data(), shaderByteCode.size(), sizeof(byte), shaderSize, shaderFile);
    if (bytesWritten != shaderSize) {
        std::cout << "Shader Loader Error: Error while loading file \"" << shaderPath << "\".\n";
        fclose(shaderFile);
        return shaderByteCode;
    }

    fclose(shaderFile);

    return shaderByteCode;
}

bool Renderer::init_direct3D11()
{
    uint32_t flags = 0;

#ifndef DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // Supported feature levels for the project
    std::vector<D3D_FEATURE_LEVEL> featureLevels = { D3D_FEATURE_LEVEL_11_0 };

    // Create temporary base device, context, and swapchain that will be promoted later.
    ComPtr<ID3D11Device> tempDevice = nullptr;
    ComPtr<ID3D11DeviceContext> tempContext = nullptr;

    // Create the device and context and return the maximum supported feature level.
    D3D_FEATURE_LEVEL maxFeatureLevel;
    if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels.data(), featureLevels.size(), D3D11_SDK_VERSION, &tempDevice, &maxFeatureLevel, &tempContext))) {
        std::cout << "D3D11 Error: Failed to initialize.\n";
        return false;
    }

    // Error out if the max feature level is below 11.0
    if (maxFeatureLevel < D3D_FEATURE_LEVEL_11_0) {
        std::cout << "D3D11 Error: Device is unable to support Feature Level 11.0.\n";
        return false;
    }
    
#ifndef DEBUG
    // Create the debug instance by querying the D3D11 device interface
    if (FAILED(tempDevice.As(&_debug))) {
        std::cout << "D3D11 Error: Failed to create debug instance.\n";
        return false;
    }

    // Create the debug info queue by querying the D3D11 debug instance
    if (FAILED(_debug.As(&_debugQueue))) {
        std::cout << "D3D11 Error: Failed to create debug info queue.\n";
        return false;
    }

    _debugQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
    _debugQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif

    // Promote the D3D11 device to ID3D11Device5
    if (FAILED(tempDevice.As(&_device))) {
        std::cout << "D3D11 Error: Failed to promote D3D11 device.\n";
        return false;
    }

    // Promote D3D11 device context to ID3D11DeviceContext4
    if (FAILED(tempContext.As(&_context))) {
        std::cout << "D3D11 Error: Failed to promote D3D11 device context.\n";
        return false;
    }
    
    return true;
}

bool Renderer::init_swapchain()
{
    if (FAILED(CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), &_factory))) {
        std::cout << "DXGI Error: Failed to create DXGI factory.\n";
        return false;
    }

    // Pulling information about the window and current monitor from GLFW that will be needed for later.
    HWND hwnd = glfwGetWin32Window(_window);

    // Set and store the swapchain format.
    _swapchain.format = DXGI_FORMAT_R8G8B8A8_UNORM;

    DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
    swapchainDesc.Width = _windowSize.width;
    swapchainDesc.Height = _windowSize.height;
    swapchainDesc.Format = _swapchain.format;
    swapchainDesc.Stereo = FALSE;
    swapchainDesc.SampleDesc.Quality = 0;
    swapchainDesc.SampleDesc.Count = 1;
    swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc.BufferCount = 3; // Triple buffering
    swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapchainDesc.Flags = 0;

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc = {};
    fullscreenDesc.RefreshRate.Numerator = 0;
    fullscreenDesc.RefreshRate.Denominator = 1;
    fullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    fullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    fullscreenDesc.Windowed = TRUE;

    ComPtr<IDXGISwapChain1> tempSwapchain = nullptr;
    if (FAILED(_factory->CreateSwapChainForHwnd(_device.Get(), hwnd, &swapchainDesc, &fullscreenDesc, nullptr, &tempSwapchain))) {
        std::cout << "DXGI Error: Failed to create swapchain." << std::endl;
        return false;
    }

    if (FAILED(tempSwapchain.As(&_swapchain.swapchain))) {
        std::cout << "DXGI Error : Failed to create swapchain." << std::endl;
        return false;
    }

    // Create backbuffer render target
    D3D11_TEXTURE2D_DESC backBufferDesc = {};
    backBufferDesc.Width = _windowSize.width;
    backBufferDesc.Height = _windowSize.height;
    backBufferDesc.MipLevels = 1;
    backBufferDesc.ArraySize = 1;
    backBufferDesc.Format = _swapchain.format;
    backBufferDesc.SampleDesc.Count = 1;
    backBufferDesc.SampleDesc.Quality = 0;
    backBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    backBufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
    backBufferDesc.CPUAccessFlags = 0;
    backBufferDesc.MiscFlags = 0;

    if (FAILED(_device->CreateTexture2D(&backBufferDesc, nullptr, &_swapchain.backBuffer.buffer))) {
        std::cout << "D3D11 Error: Failed to create back buffer texture." << std::endl;
        return false;
    }
    if (FAILED(_swapchain.swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)_swapchain.backBuffer.buffer.GetAddressOf()))) {
        std::cout << "D3D11 Error: Failed to get back buffer texture." << std::endl;
        return false;
    }
    if (FAILED(_device->CreateRenderTargetView(_swapchain.backBuffer.buffer.Get(), nullptr, &_swapchain.backBuffer.RTV))) {
        std::cout << "D3D11 Error: Failed to create render target for back buffer." << std::endl;
        return false;
    }
    
    return true;
}

bool Renderer::init_g_buffer()
{
    // Create depth stencil buffer
    D3D11_TEXTURE2D_DESC depthStencilDesc = {};
    depthStencilDesc.Width = _windowSize.width;
    depthStencilDesc.Height = _windowSize.height;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    if (FAILED(_device->CreateTexture2D(&depthStencilDesc, nullptr, &_gBuffer.depthBuffer.buffer))) {
        std::cout << "D3D11 Error: Failed to create depth stencil buffer." << std::endl;
        return false;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilDSVDesc = {};
    DepthStencilDSVDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DepthStencilDSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    DepthStencilDSVDesc.Texture2D.MipSlice = 0;
    DepthStencilDSVDesc.Flags = 0;

    if (FAILED(_device->CreateDepthStencilView(_gBuffer.depthBuffer.buffer.Get(), &DepthStencilDSVDesc, &_gBuffer.depthBuffer.DSV))) {
        std::cout << "D3D11 Error: Failed to create DSV for depth stencil buffer." << std::endl;
        return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC DepthStencilSRVDesc = {};
    DepthStencilSRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    DepthStencilSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    DepthStencilSRVDesc.Texture2D.MostDetailedMip = 0;
    DepthStencilSRVDesc.Texture2D.MipLevels = -1;

    if (FAILED(_device->CreateShaderResourceView(_gBuffer.depthBuffer.buffer.Get(), &DepthStencilSRVDesc, &_gBuffer.depthBuffer.SRV))) {
        std::cout << "D3D11 Error: Failed to create SRV for depth stencil buffer." << std::endl;
        return false;
    }

    // Create vertex normal and UV cordinate buffer
    D3D11_TEXTURE2D_DESC vertNormUVCordDesc = {};
    vertNormUVCordDesc.Width = _windowSize.width;
    vertNormUVCordDesc.Height = _windowSize.height;
    vertNormUVCordDesc.MipLevels = 1;
    vertNormUVCordDesc.ArraySize = 1;
    vertNormUVCordDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    vertNormUVCordDesc.SampleDesc.Count = 1;
    vertNormUVCordDesc.SampleDesc.Quality = 0;
    vertNormUVCordDesc.Usage = D3D11_USAGE_DEFAULT;
    vertNormUVCordDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    vertNormUVCordDesc.CPUAccessFlags = 0;
    vertNormUVCordDesc.MiscFlags = 0;

    if (FAILED(_device->CreateTexture2D(&vertNormUVCordDesc, nullptr, &_gBuffer.vertNormalUVCord.buffer))) {
        std::cout << "D3D11 Error: Failed to create texture for G-buffer." << std::endl;
        return false;
    }
    if (FAILED(_device->CreateRenderTargetView(_gBuffer.vertNormalUVCord.buffer.Get(), nullptr, &_gBuffer.vertNormalUVCord.RTV))) {
        std::cout << "D3D11 Error: Failed to create RTV for G-buffer." << std::endl;
        return false;
    }
    if (FAILED(_device->CreateShaderResourceView(_gBuffer.vertNormalUVCord.buffer.Get(), nullptr, &_gBuffer.vertNormalUVCord.SRV))) {
        std::cout << "D3D11 Error: Failed to create SRV for G-buffer." << std::endl;
        return false;
    }

    // Create materialID buffer
    D3D11_TEXTURE2D_DESC materialIDDesc = {};
    materialIDDesc.Width = _windowSize.width;
    materialIDDesc.Height = _windowSize.height;
    materialIDDesc.MipLevels = 1;
    materialIDDesc.ArraySize = 1;
    materialIDDesc.Format = DXGI_FORMAT_R8_UINT;
    materialIDDesc.SampleDesc.Count = 1;
    materialIDDesc.SampleDesc.Quality = 0;
    materialIDDesc.Usage = D3D11_USAGE_DEFAULT;
    materialIDDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    materialIDDesc.CPUAccessFlags = 0;
    materialIDDesc.MiscFlags = 0;

    if (FAILED(_device->CreateTexture2D(&materialIDDesc, nullptr, &_gBuffer.materialID.buffer))) {
        std::cout << "D3D11 Error: Failed to create texture for G-buffer." << std::endl;
        return false;
    }
    if (FAILED(_device->CreateRenderTargetView(_gBuffer.materialID.buffer.Get(), nullptr, &_gBuffer.materialID.RTV))) {
        std::cout << "D3D11 Error: Failed to create RTV for G-buffer." << std::endl;
        return false;
    }
    if (FAILED(_device->CreateShaderResourceView(_gBuffer.materialID.buffer.Get(), nullptr, &_gBuffer.materialID.SRV))) {
        std::cout << "D3D11 Error: Failed to create SRV for G-buffer." << std::endl;
        return false;
    }

    std::vector<ID3D11RenderTargetView*> renderTargets = {
        _swapchain.backBuffer.RTV.Get(),
        _gBuffer.vertNormalUVCord.RTV.Get(),
        _gBuffer.materialID.RTV.Get()
    };

    _context->OMSetRenderTargets(renderTargets.size(), renderTargets.data(), _gBuffer.depthBuffer.DSV.Get());

    return true;
}

bool Renderer::init_shaders()
{   
    // Init Vertex Shaders
    
    VertexInputLayout inputLayout = StaticVertices::get_layout();
    ShaderByteCode staticVertexShader = load_compiled_shader("../x64/Debug/VertexShader.vert.cso");
    
    if (FAILED(_device->CreateInputLayout(inputLayout.data(), inputLayout.size(), staticVertexShader.data(), staticVertexShader.size(), &_shaders.inputLayouts.staticVertices))) {
        std::cout << "D3D11 Error: Failed to create input layout.\n";
        return false;
    }
    if (FAILED(_device->CreateVertexShader(staticVertexShader.data(), staticVertexShader.size(), nullptr, &_shaders.vertexShaders.staticVertexShader))) {
        std::cout << "D3D11 Error: Failed to create vertex shader.\n";
        return false;
    }

    return true;
}

bool Renderer::init()
{
    glfwGetWindowSize(_window, &_windowSize.width, &_windowSize.height);

    if (!init_direct3D11())
        return false;
    if (!init_swapchain())
        return false;
    if (!init_g_buffer())
        return false;
    if (!init_shaders())
        return false;

    D3D11_DEPTH_STENCIL_DESC DSDesc = {};
    DSDesc.DepthEnable = FALSE;
    DSDesc.StencilEnable = FALSE;
    
    if (FAILED(_device->CreateDepthStencilState(&DSDesc, &_depthStencilState))) {
        std::cout << "D3D11 Error: Failed to create depth stencil state.\n";
        return false;
    }

    std::vector<StaticVertices> vertices = {
        { DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), Colors::White },
        { DirectX::XMFLOAT3(-1.0f,  1.0f, -1.0f), Colors::Black },
        { DirectX::XMFLOAT3( 1.0f,  1.0f, -1.0f), Colors::Red },
        { DirectX::XMFLOAT3( 1.0f, -1.0f, -1.0f), Colors::Green },
        { DirectX::XMFLOAT3(-1.0f, -1.0f,  1.0f), Colors::Blue },
        { DirectX::XMFLOAT3(-1.0f,  1.0f,  1.0f), Colors::Yellow },
        { DirectX::XMFLOAT3( 1.0f,  1.0f,  1.0f), Colors::Cyan },
        { DirectX::XMFLOAT3( 1.0f, -1.0f,  1.0f), Colors::Magenta}
    };

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = sizeof(StaticVertices) * vertices.size();
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexBufferSubResource = {};
    vertexBufferSubResource.pSysMem = vertices.data();

    if (FAILED(_device->CreateBuffer(&vertexBufferDesc, &vertexBufferSubResource, &_vertexBuffer))) {
        std::cout << "D3D11 Error: Failed to create vertex buffer.\n";
        return false;
    }

    std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 7, 0, 7, 8, 0, 8, 1 };

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.ByteWidth = sizeof(uint32_t) * indices.size();
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA indexBufferSubResource = {};
    indexBufferSubResource.pSysMem = indices.data();

    if (FAILED(_device->CreateBuffer(&indexBufferDesc, &indexBufferSubResource, &_indexBuffer))) {
        std::cout << "D3D11 Error: Failed to create index buffer.\n";
        return false;
    }

    return true;
}

void Renderer::draw()
{
    _context->OMSetRenderTargets(1, _swapchain.backBuffer.RTV.GetAddressOf(), _gBuffer.depthBuffer.DSV.Get());
    _context->OMSetDepthStencilState(_depthStencilState.Get(), 0);

    float clearColor[4] = { 0.0, 0.0, 1.0, 1.0 };
    _context->ClearRenderTargetView(_swapchain.backBuffer.RTV.Get(), clearColor);
    _context->ClearDepthStencilView(_gBuffer.depthBuffer.DSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = (float)_windowSize.width;
    viewport.Height = (float)_windowSize.height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    
    _context->RSSetViewports(1, &viewport);

    _context->IASetInputLayout(_shaders.inputLayouts.staticVertices.Get());

    uint32_t stride = sizeof(StaticVertices), offset = 0;
    _context->IASetVertexBuffers(0, 1, _vertexBuffer.GetAddressOf(), &stride, &offset);
    _context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _context->VSSetShader(_shaders.vertexShaders.staticVertexShader.Get(), nullptr, 0);
    _context->PSSetShader(_pixelShader.Get(), nullptr, 0);

    _context->Draw(3, 0);

    _swapchain.swapchain->Present(0, 0);
}

VertexInputLayout StaticVertices::get_layout()
{
    VertexInputLayout inputLayoutDesc = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        //{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        //{"TEXCORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    return inputLayoutDesc;
}
