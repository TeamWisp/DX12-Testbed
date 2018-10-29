#pragma once

#include "Window.h"
#include "Renderer.h"

class Instance
{
public:
	Instance();
	~Instance();

	void Init();
	void Loop();
	void Cleanup();

protected:
	Window * window;
	renderer::Renderer* render;
};

