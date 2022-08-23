#pragma once

#include <string>

#include "../Platform/OpenXRManager.h"
#include "../Graphics/Graphics.h"

#include <thread> // sleep_for
#include <vector>
#include <algorithm> // any_of


#include "../Graphics/IndexedTriangleList.h"
#include "../Graphics/VertexBuffer.h"
#include "../Graphics/IndexBuffer.h"
#include "../Graphics/VertexShader.h"
#include "../Graphics/PixelShader.h"
#include "../Graphics/InputLayout.h"

namespace Katame 
{

	class Application
	{
	public:
		Application();
		~Application();

		void Launch();

	private:
		Graphics* gfx() { return openxrManager->gfx; };
		OpenXRManager* openxrManager;
	public:

		void Draw( XrCompositionLayerProjectionView& view );
		void Update();
		void Update_Predicted();

	private:
		bool m_Running = true, m_Minimized = false;
		bool xr_running = false;

		

		std::vector<XrPosef> app_cubes;


		VertexBuffer* app_vertex_buffer = nullptr;
		IndexBuffer* app_index_buffer = nullptr;
		VertexShader* app_vshader = nullptr;
		PixelShader* app_pshader = nullptr;
		InputLayout* app_shader_layout = nullptr;
		//ID3D11VertexShader* app_vshader = nullptr;
		//ID3D11PixelShader* app_pshader = nullptr;
		//ID3D11InputLayout* app_shader_layout = nullptr;
		ID3D11Buffer* app_constant_buffer = nullptr;
		//ID3D11Buffer* app_vertex_buffer = nullptr;
		//ID3D11Buffer* app_index_buffer = nullptr;

	};

}
