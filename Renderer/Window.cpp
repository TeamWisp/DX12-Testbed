#include "Window.h"

#include <locale>
#include <codecvt>

IDXGIFactory5* Window::factory = nullptr;
std::vector<Window::DisplayAdapter> Window::adapters = std::vector<Window::DisplayAdapter>();
std::map<HWND, Window*> Window::windows = std::map<HWND, Window*>();
DXGI_FORMAT Window::backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;

std::vector<Window::DisplayAdapter> Window::GetDisplayAdapters()
{
	if (adapters.size() > 0)
		return adapters;

	UINT n = 0;
	IDXGIAdapter* adapter = nullptr;
	std::vector<DisplayAdapter> adapterList;
	if (factory == nullptr) {
#ifdef NDEBUG
		HRESULT hr = CreateDXGIFactory2(NULL, __uuidof(IDXGIFactory5), (void**)(&factory));
#endif
#ifndef NDEBUG
		HRESULT hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, __uuidof(IDXGIFactory5), (void**)(&factory));
#endif
	}

	while (factory->EnumAdapters(n, &adapter) != DXGI_ERROR_NOT_FOUND) {
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);
		
		DisplayAdapter displayAdapter = {};
		displayAdapter.adapter = adapter;
		displayAdapter.description = desc;

		std::vector<DisplayWindow> windows;

		UINT i = 0;
		IDXGIOutput* output = nullptr;
		while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND) {
			DisplayWindow window = {};
			window.output = output;

			DXGI_OUTPUT_DESC desc;
			output->GetDesc(&desc);

			window.description = desc;

			UINT j = 0;

			window.output->GetDisplayModeList(backBufferFormat, 0, &j, nullptr);

			std::vector<DXGI_MODE_DESC> descriptors(j);

			window.output->GetDisplayModeList(backBufferFormat, 0, &j, descriptors.data());

			std::vector<DisplayMode> modes;

			for (int i = j - 1; i >= 0; i--) {
				DisplayMode mode = {};
				mode.width = descriptors[i].Width;
				mode.height = descriptors[i].Height;
				mode.refreshRate = descriptors[i].RefreshRate.Numerator / descriptors[i].RefreshRate.Denominator;

				mode.output = window.output;
				mode.adapter = window.adapter;

				mode.description = descriptors[i];

				modes.push_back(mode);
			}

			window.adapter = adapter;

			window.displayModes = modes;
			
			windows.push_back(window);

			i++;
		}

		displayAdapter.windows = windows;
		
		adapterList.push_back(displayAdapter);

		n++;
	}

	adapters = adapterList;

	return adapters;
}

void Window::Cleanup()
{
	for (size_t i = 0; i < adapters.size(); ++i) {
		for (size_t j = 0; j < adapters[i].windows.size(); ++j) {
			adapters[i].windows[j].output->Release();
		}
		adapters[i].adapter->Release();
	}
	adapters.clear();

	if (factory != nullptr)
		factory->Release();
}

Window::Window(DisplayMode mode, std::wstring name, bool fullscreen)
{
	inputClass = new Input();

	this->mode = mode;
	this->fullscreen = fullscreen;

	shouldClose = false;

	windowName = name;

	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;

	std::string str = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(windowName);

	instance = GetModuleHandle(NULL);

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = MsgHandler;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = instance;
	wc.hIcon = LoadIcon(instance, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(instance, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = str.c_str();
	wc.cbSize = sizeof(WNDCLASSEX);

	RegisterClassEx(&wc);

	DWORD err = GetLastError();

	ZeroMemory(&dmScreenSettings, sizeof(DEVMODE));

	posX = (GetSystemMetrics(SM_CXSCREEN) - mode.width) / 2;
	posY = (GetSystemMetrics(SM_CYSCREEN) - mode.height) / 2;

	if (fullscreen) {
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = mode.width;
		dmScreenSettings.dmPelsHeight = mode.height;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		posX = posY = 0;

		windowHandle = CreateWindowEx(WS_EX_APPWINDOW, str.c_str(), str.c_str(),
			WS_POPUP,
			posX, posY, mode.width, mode.height, NULL, NULL, instance, NULL);
	}
	else {

		windowHandle = CreateWindowEx(WS_EX_APPWINDOW, str.c_str(), str.c_str(),
			WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			posX, posY, mode.width, mode.height, NULL, NULL, instance, NULL);
	}

	err = GetLastError();

	windows[windowHandle] = this;

	ShowWindow(windowHandle, SW_SHOW);
	SetForegroundWindow(windowHandle);
	SetFocus(windowHandle);

	ShowCursor(true);
}

Window::~Window()
{
	delete inputClass;

	windows.erase(windowHandle);
	if (fullscreen) {
		ChangeDisplaySettings(NULL, 0);
	}

	DestroyWindow(windowHandle);
	windowHandle = NULL;

	std::string str = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(windowName);
	UnregisterClass(str.c_str(), instance);

	instance = NULL;
}

LRESULT Window::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg) {
	case WM_KEYDOWN:
		inputClass->SetKeyState((unsigned int)wparam, true);
		return 0;
		break;
	case WM_KEYUP:
		inputClass->SetKeyState((unsigned int)wparam, false);
		return 0;
		break;
	case WM_DESTROY:
		shouldClose = true;
		return 0;
		break;
	case WM_CLOSE:
		shouldClose = true;
		return 0;
		break;
	default:
		return DefWindowProc(hwnd, umsg, wparam, lparam);
		break;
	}
}

void Window::HandleMessages()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	if (PeekMessage(&msg, windowHandle, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (msg.message == WM_QUIT) {
		shouldClose = true;
	}
}

bool Window::ShouldClose()
{
	return shouldClose;
}

Input * Window::GetInput()
{
	return inputClass;
}

Window::DisplayMode Window::GetDisplayMode()
{
	return mode;
}

HWND Window::GetWindowhandle()
{
	return windowHandle;
}

IDXGIFactory5 * Window::GetFactory()
{
	if (factory == nullptr) {
#ifdef NDEBUG
		HRESULT hr = CreateDXGIFactory2(NULL, __uuidof(IDXGIFactory5), (void**)(&factory));
#endif
#ifndef NDEBUG
		HRESULT hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, __uuidof(IDXGIFactory5), (void**)(&factory));
#endif
	}
	return factory;
}

void Window::SetBackBufferFormat(DXGI_FORMAT format)
{
	backBufferFormat = format;
}

DXGI_FORMAT Window::GetBackBufferFormat()
{
	return backBufferFormat;
}

LRESULT MsgHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg) {

	default:
		if (Window::windows[hwnd] != nullptr) {
			return Window::windows[hwnd]->MessageHandler(hwnd, umsg, wparam, lparam);
		}
		else {
			return DefWindowProc(hwnd, umsg, wparam, lparam);
		}
		break;
	}
}
