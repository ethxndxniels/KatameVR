#pragma once

#include <string>

#include "../Graphics/Graphics.h"
#include "../XR/XRCore.h"
#include "../Renderer/Renderer.h"
#include "../Platform/Win32Window.h"

#include <thread> // sleep_for
#include <vector>
#include <algorithm> // any_of

#include "../Utilities/FrameTimer.h"

#include "../Drawable/Drawables/ColorCube.h"
#include "../Drawable/Drawables/NormalCube.h"

#include "../Renderer/Mesh.h"

#include "../Lights/DirLight.h"

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
		XRCore* m_XRCore = nullptr;
		Graphics* m_Graphics = nullptr;
		Renderer* m_Renderer = nullptr;
		Win32Window* m_Window = nullptr;
	private:
		ColorCube* m_Cube;
		ColorCube* m_Cube2;
		NormalCube* m_NCube;
		NormalCube* m_NCube2;
		Mesh* m_Mesh;
		Mesh* m_Mesh2;
	private:
		DirLight* m_DirLight;
	private:
		FrameTimer m_Timer;
		float speed_factor = 1.0f;
	private:
		bool m_Running = true;
		bool requestRestart = false;
	};
}
