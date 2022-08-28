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
		Graphics* gfx() { return openxrManager->gfx; };
		OpenXRManager* openxrManager;
	public:
		void Draw( XrCompositionLayerProjectionView& view );
		void Update( float dt );
		void Update_Predicted();
	private:
		bool m_Running = true, m_Minimized = false;
		bool xr_running = false;
		FrameTimer timer;
		float speed_factor = 1.0f;
		Mesh* m_Model = nullptr;
		DirectX::XMFLOAT4 modelOrientation = { 0.0f, 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 modelPosition = { 1.0f, -5.0f, 1.0f };

		std::vector<XrPosef> app_cubes;
		Bind::VertexBuffer* app_vertex_buffer = nullptr;
		Bind::IndexBuffer* app_index_buffer = nullptr;
		Bind::VertexShader* app_vshader = nullptr;
		Bind::PixelShader* app_pshader = nullptr;
		Bind::InputLayout* app_shader_layout = nullptr;
		Bind::Topology* app_topology = nullptr;
		Bind::VertexShader* phongVS = nullptr;
		Bind::PixelShader* phongPS = nullptr;
		Bind::Rasterizer* rasterizer = nullptr;
		Bind::Blender* blender = nullptr;

		struct DirectionalLight
		{
			DirectX::XMFLOAT4 lightDir = { 0.0f, -1.0f, 0.0f, 0.0f };
		};
		ID3D11Buffer* dir_light_cbuffer = nullptr;

		DirectionalLight m_StructuredBufferData;
		Bind::PixelConstantBuffer<DirectionalLight>* dirLight = nullptr;
	
		struct app_transform_buffer_t 
		{
			DirectX::XMFLOAT4X4 world;
			DirectX::XMFLOAT4X4 viewproj;
		};
		ID3D11Buffer* app_constant_buffer = nullptr;
	};
}
