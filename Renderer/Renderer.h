#pragma once
#include "Window.h"

namespace renderer {

	class Renderer
	{
	public:
		Renderer(Window* window);
		virtual ~Renderer();

		virtual int Initialize() = 0;

		virtual void RenderStart() = 0;
		virtual void RenderEnd() = 0;

	protected:
		Window * window;
	};

}