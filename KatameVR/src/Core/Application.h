#pragma once

#include <string>

#include <thread>
#include <vector>
#include <algorithm>

#include "../Platform/XRCore.h"
#include "../Platform/XRInput.h"
#include "../Platform/XRHandTracking.h"
#include "../Platform/XRRender.h"
#include "../Platform/XRGraphics.h"

#include "../Platform/XROculusTouchDefines.h"

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
		//void OnEvent( Event* e );
		void Update( float dt );
		void Update_Predicted();
	private:
		bool m_Running = true, m_Minimized = false;
		bool xr_running = false;
		FrameTimer timer;
		float speed_factor = 1.0f;
		
		// Actions
		XrAction m_Action_SwitchScene, m_Action_Haptic;
		XrAction m_Action_PoseLeft, m_Action_PoseRight;
		XrActionStateBoolean m_ActionState_SwitchScene;
		XrActionStatePose m_ActionState_PoseLeft;
		XrActionStatePose m_ActionState_PoseRight;
		std::vector<XrActionSuggestedBinding> m_SuggestedBindings;
	};
}
