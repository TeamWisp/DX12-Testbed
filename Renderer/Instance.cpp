#include "Instance.h"
#include "Defines.h"
#include "DX12Renderer.h"
#include <iostream>


Instance::Instance()
{	
}


Instance::~Instance()
{
}

void Instance::Init()
{
	std::vector<Window::DisplayAdapter> adapters = Window::GetDisplayAdapters();

	Window::DisplayMode mode = {};

	for (size_t i = 0, size = adapters.size(); i < size; ++i) {
		std::wcout << L"Found adapter " << i << L": " << adapters[i].description.Description << std::endl;
		std::wcout << L"\t" << L"Video memory: " << adapters[i].description.DedicatedVideoMemory / 1024 / 1024 << L"MB" << std::endl;
		std::wcout << L"\t" << L"Attached displays:" << std::endl;
		std::vector<Window::DisplayWindow> windows = adapters[i].windows;
		for (size_t j = 0, size2 = windows.size(); j < size2; ++j) {
			std::wcout << L"\t\t" << windows[j].description.DeviceName << std::endl;
			std::wcout << L"\t\t" << L"Available display modes:" << std::endl;
			std::vector<Window::DisplayMode> modes = windows[j].displayModes;
			for (size_t k = 0, size3 = modes.size(); k < size3; ++k) {
				std::wcout << L"\t\t\t" << L"Width: " << modes[k].width <<
					L", Height: " << modes[k].height <<
					L", Refresh rate: " << modes[k].refreshRate << std::endl;
				if (modes[k].width == 1024 && modes[k].height == 768)
					mode = modes[k];
			}
		}
	}

#ifdef FULLSCREEN
	window = new Window(adapters[0].windows[0].displayModes[0], L"Raymarching Shadows", true);
#endif
#ifndef FULLSCREEN
	window = new Window(mode, L"Raymarching Shadows", false);
#endif

	render = new renderer::dx12::DX12Renderer(window);

	if (render->Initialize() != 0) {
		std::cout << "[FATAL] Initializing renderer failed" << std::endl;
	}
}

void Instance::Loop()
{
	while (!window->ShouldClose() && !window->GetInput()->GetKeyState(VK_ESCAPE)) {

		render->RenderStart();

		render->RenderEnd();

		window->HandleMessages();
	}
}

void Instance::Cleanup()
{
	delete render;

	delete window;
	Window::Cleanup();
}

int main(int argc, char** argv) {
	Instance* instance = new Instance();

	instance->Init();
	instance->Loop();
	instance->Cleanup();

	delete instance;
	//getchar();
	return 0;
}