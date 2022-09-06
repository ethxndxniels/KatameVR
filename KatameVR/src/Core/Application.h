#pragma once

#include <string>

#include "../XR/XRCore.h"
#include "../Graphics/Graphics.h"

#include <thread> // sleep_for
#include <vector>
#include <algorithm> // any_of


#include "../Graphics/IndexedTriangleList.h"
#include "../Bindables/VertexBuffer.h"
#include "../Bindables/IndexBuffer.h"
#include "../Bindables/VertexShader.h"
#include "../Bindables/PixelShader.h"
#include "../Bindables/InputLayout.h"
#include "../Bindables/Topology.h"
#include "../Bindables/ConstantBuffers.h"
#include "../Bindables/Rasterizer.h"
#include "../Bindables/Blender.h"

#include "../Graphics/Mesh.h"
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
