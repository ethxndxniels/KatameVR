#pragma once

#include <string>

#include "../Graphics/Graphics.h"
#include "../XR/XRCore.h"
#include "../Renderer/Renderer.h"

#include <thread> // sleep_for
#include <vector>
#include <algorithm> // any_of

#include "../Utilities/FrameTimer.h"

#include "../Drawable/Drawables/Cube.h"

namespace Katame 
{
	class Application
	{
	public:
		Application();
		~Application();
		void Launch();
	private:
		XRCore* m_XRCore = nullptr;
		Graphics* m_Graphics = nullptr;
		Renderer* m_Renderer = nullptr;
	private:
		Cube* m_Cube;
	private:
		bool m_Running = true;
		bool requestRestart = false;
	};
}
