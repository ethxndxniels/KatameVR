#pragma once

#include <string>

#include <thread>
#include <vector>
#include <algorithm>

#include "../Platform/XRCore.h"

#include "../Utilities/FrameTimer.h"

#include <DirectXMath.h>

namespace Katame 
{
	class Application
	{
	public:
		Application();
		~Application();
		void Launch();
	public:
		void Draw( XrCompositionLayerProjectionView& view );
		void Update( float dt );
		void Update_Predicted();
	private:
		bool m_Running = true, m_Minimized = false;
		bool xr_running = false;
		FrameTimer timer;
		float speed_factor = 1.0f;
		



	};
}
