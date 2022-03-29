#include "Renderer.h"

bool Renderer::init_dxgi_factory()
{
    if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), &_factory))) {
        std::cout << "DXGI Error: Failed to create DXGI factory." << std::endl;
        return false;
    }

    return true;
}

bool Renderer::init_adapter()
{
    // Enumerate adapters into a vector of adapters
    ComPtr<IDXGIAdapter> tempAdapter;
    std::vector<ComPtr<IDXGIAdapter>> adapters;
    while (SUCCEEDED(_factory->EnumAdapters(adapters.size(), &tempAdapter)))
        adapters.push_back(tempAdapter);

    if (adapters.size() == 0) {
        std::cout << "DXGI Error: No valid adapters avaliable." << std::endl;
        return false;
    }

    // List avaliable adapters by name
    std::cout << "Avaliable Adapters: " << std::endl;
    for (uint32_t i = 0; i < adapters.size(); ++i) {
        DXGI_ADAPTER_DESC desc;
        adapters[i]->GetDesc(&desc);
        std::wcout << i + 1 << ") " << desc.Description << std::endl;
    }

    // Allow the user to select an adapter
    std::cout << "\nPlease select an adapter: ";

    uint32_t userSelect;
    std::cin >> userSelect;
    while (userSelect < 1 || userSelect > adapters.size()) {
        std::cout << "\nPlease select a valid adapter: ";
        std::cin >> userSelect;
    }

    _adapter = adapters[userSelect - 1];

    return true;
}

bool Renderer::init_device_and_context()
{
    uint32_t flags = 0;

#ifndef DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // Create the device, the minimum supported feature level for this project is 11.0
    D3D_FEATURE_LEVEL maxFeatureLevel;
    std::vector<D3D_FEATURE_LEVEL> featureLevels = { D3D_FEATURE_LEVEL_11_0 };
    if (FAILED(D3D11CreateDevice(_adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, flags, featureLevels.data(), featureLevels.size(), D3D11_SDK_VERSION, &_device, &maxFeatureLevel, &_context))) {
        std::cout << "D3D11 Error: Failed to create device and/or context." << std::endl;
        return false;
    }

    // Error out if the max feature level is below 11.0
    if (maxFeatureLevel < D3D_FEATURE_LEVEL_11_0) {
        std::cout << "D3D11 Error: Device is unable to support Feature Level 11.0." << std::endl;
        return false;
    }
    
#ifndef DEBUG
    // Create the debug instance by querying the D3D11 device interface
    if (FAILED(_device.As(&_debug))) {
        std::cout << "D3D11 Error: Failed to create debug instance." << std::endl;
        return false;
    }

    // Create the debug info queue by querying the D3D11 debug instance
    if (FAILED(_debug.As(&_debugQueue))) {
        std::cout << "D3D11 Error: Failed to create debug info queue." << std::endl;
        return false;
    }

    _debugQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
    _debugQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
    
    return true;
}

bool Renderer::init_swapchain()
{
    ComPtr<IDXGIOutput> output;
    if (FAILED(_adapter->EnumOutputs(0, &output))) {
        std::cout << "DXGI Error: Failed to get output." << std::endl;
        return false;
    }

    // Pull the refresh rate of the active monitor
    GLFWmonitor* monitor = glfwGetWindowMonitor(_window.get());
    const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);

    DXGI_MODE_DESC desiredMode = {};
    desiredMode.Width = _windowSize.width;
    desiredMode.Height = _windowSize.height;
    desiredMode.RefreshRate.Numerator = videoMode->refreshRate;
    desiredMode.RefreshRate.Denominator = 1;
    desiredMode.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desiredMode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    desiredMode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    DXGI_MODE_DESC closestMode;
    output->FindClosestMatchingMode(&desiredMode, &closestMode, _device.Get());

    DXGI_SWAP_CHAIN_DESC desc = {};
    desc.BufferDesc = closestMode;
    desc.Windowed = TRUE;
    desc.OutputWindow = glfwGetWin32Window(_window.get());
    desc.SampleDesc.Count = 1;
    desc.BufferCount = 1;

    return true;
}

bool Renderer::init()
{
    glfwGetWindowSize(_window.get(), &_windowSize.width, &_windowSize.height);

    if (!init_dxgi_factory())
        return false;
    if (!init_adapter())
        return false;
    if (!init_device_and_context())
        return false;
    if (!init_swapchain())
        return false;

    return true;
}
