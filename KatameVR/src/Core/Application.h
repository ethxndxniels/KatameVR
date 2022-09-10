#pragma once

#include <string>

#include "../XR/XRCore.h"
#include "../Graphics/Graphics.h"

#include <thread> // sleep_for
#include <vector>
#include <algorithm> // any_of

#include "../Utilities/FrameTimer.h"

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
	private:
		bool m_Running = true;
		bool requestRestart = false;
	};
}
