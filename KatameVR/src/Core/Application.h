#pragma once

#include <string>

#include "../Graphics/Graphics.h"
#include "../XR/XRCore.h"
#include "../Renderer/Renderer.h"
#include "../Platform/Win32Window.h"

#include <thread> // sleep_for
#include <vector>
#include <algorithm> // any_of

#include "../UI/ImguiHandler.h"

#include "../Utilities/FrameTimer.h"

#include "../Drawable/Drawables/ColorCube.h"
#include "../Drawable/Drawables/NormalCube.h"

#include "../Drawable/Drawables/Hands.h"

#include "../Renderer/Model.h"

#include "../Lights/DirLight.h"
#include "../Lights/PointLight.h"

namespace Katame 
{
	class Application
	{
	public:
		Application();
		~Application();
		void Launch();
		void Update( float dt );
		void Submit();
	private:
		ImguiHandler imgui;
		XRCore* m_XRCore = nullptr;
		Graphics* m_Graphics = nullptr;
		Renderer* m_Renderer = nullptr;
		Win32Window* m_Window = nullptr;
	private:
		NormalCube* m_NCube;
		Model* m_Sponza;
		Hands* m_Hands;
	private:
		DirLight* m_DirLight;
		PointLight* m_PointLight;
	private:
		FrameTimer m_Timer;
		float speed_factor = 1.0f;
	private:
		bool m_Running = true;
		bool requestRestart = false;
	};
}
