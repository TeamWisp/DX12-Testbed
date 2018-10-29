#pragma once

#include "Input.h"

#include <vector>
#include <Windows.h>

#include <dxgi1_6.h>

#pragma comment(lib, "dxgi.lib")

class Window
{
public:
	struct DisplayMode;
	struct DisplayWindow;
	struct DisplayAdapter;

	struct DisplayAdapter {
		IDXGIAdapter* adapter;
		DXGI_ADAPTER_DESC description;
		std::vector<DisplayWindow> windows;
	};

	struct DisplayWindow {
		IDXGIAdapter* adapter;
		IDXGIOutput* output;
		DXGI_OUTPUT_DESC description;
		std::vector<DisplayMode> displayModes;
	};

	struct DisplayMode {
		IDXGIOutput* output;
		IDXGIAdapter* adapter;
		int width, height;
		int refreshRate;
		DXGI_MODE_DESC description;
	};

	static std::vector<DisplayAdapter> GetDisplayAdapters();

	static void Cleanup();

	Window(DisplayMode mode, std::wstring name, bool fullscreen);
	~Window();

	LRESULT CALLBACK MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

	void HandleMessages();

	bool ShouldClose();

	Input* GetInput();

	DisplayMode GetDisplayMode();

	HWND GetWindowhandle();

	static std::map<HWND, Window*> windows;

	static IDXGIFactory5* GetFactory();

	static void SetBackBufferFormat(DXGI_FORMAT format);

	static DXGI_FORMAT GetBackBufferFormat();

protected:
	static IDXGIFactory5 * factory;

	static std::vector<DisplayAdapter> adapters;

	static DXGI_FORMAT backBufferFormat;

	Input* inputClass;

	HWND windowHandle;
	HINSTANCE instance;
	std::wstring windowName;

	bool shouldClose;

	DisplayMode mode;

	bool fullscreen;
};

static LRESULT CALLBACK MsgHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);