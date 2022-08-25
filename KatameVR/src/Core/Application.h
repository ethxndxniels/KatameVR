#pragma once

#include <string>

#include "../Platform/OpenXRManager.h"
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

#include "../Graphics/Mesh.h"

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

		Model* m_Model = nullptr;

		std::vector<XrPosef> app_cubes;


		Bind::VertexBuffer* app_vertex_buffer = nullptr;
		Bind::IndexBuffer* app_index_buffer = nullptr;
		Bind::VertexShader* app_vshader = nullptr;
		Bind::PixelShader* app_pshader = nullptr;
		Bind::InputLayout* app_shader_layout = nullptr;
		//ID3D11VertexShader* app_vshader = nullptr;
		//ID3D11PixelShader* app_pshader = nullptr;
		//ID3D11InputLayout* app_shader_layout = nullptr;
		ID3D11Buffer* app_constant_buffer = nullptr;
		//ID3D11Buffer* app_vertex_buffer = nullptr;
		//ID3D11Buffer* app_index_buffer = nullptr;

	};

}
