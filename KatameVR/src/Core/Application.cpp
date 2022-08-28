#include "Application.h"
#include "Log.h"

#include "../Geometry/Cube.h"
#include "../Bindables/Sampler.h"

namespace Katame
{
	Application::Application()
	{
		openxrManager = new OpenXRManager();

		auto cube = Cube::MakeIndependent( std::move(
			Dvtx::VertexLayout{}
			.Append( Dvtx::VertexLayout::Position3D )
			.Append( Dvtx::VertexLayout::Normal )
		) );
		cube.Transform( DirectX::XMMatrixScaling( 5.0f, 5.0f, 5.0f ) );
		cube.SetNormalsIndependentFlat();

		app_vertex_buffer = new Bind::VertexBuffer( gfx(), cube.vertices );
		app_index_buffer = new Bind::IndexBuffer( gfx(), cube.indices );
		app_vshader = new Bind::VertexShader( gfx(), "./Shaders/Bin/VertexShader.cso" );
		app_pshader = new Bind::PixelShader( gfx(), "./Shaders/Bin/PixelShader.cso" );
		app_shader_layout = new Bind::InputLayout( gfx(), cube.vertices.GetLayout(), app_vshader->GetBytecode());
		app_topology = new Bind::Topology( gfx(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		CD3D11_BUFFER_DESC     const_buff_desc( sizeof( app_transform_buffer_t ), D3D11_BIND_CONSTANT_BUFFER );
		gfx()->m_Device->CreateBuffer(&const_buff_desc, nullptr, &app_constant_buffer);
		
		phongVS = new Bind::VertexShader( gfx(), "./Shaders/Bin/PhongVS.cso" );
		phongPS = new Bind::PixelShader( gfx(), "./Shaders/Bin/PhongPS.cso" );
		m_Model = new Mesh( "Models\\petty_imp\\untitled.fbx", gfx(), phongVS );
	}

	Application::~Application()
	{
	}

	void Application::Launch()
	{
		KM_CORE_INFO( "Launching Application.." );
		while (m_Running) 
		{
			openxrManager->openxr_poll_events( m_Running, xr_running );
			if (xr_running) {
				openxrManager->openxr_poll_actions();
				Update();
				openxrManager->openxr_render_frame( this );
				if (openxrManager->get_session_state() != XR_SESSION_STATE_VISIBLE &&
					openxrManager->get_session_state() != XR_SESSION_STATE_FOCUSED) {
					std::this_thread::sleep_for( std::chrono::milliseconds( 250 ) );
				}
			}
		}
		openxrManager->openxr_shutdown();
		delete openxrManager;
	}

	void Application::Draw( XrCompositionLayerProjectionView& view ) 
	{
		// Set up camera matrices based on OpenXR's predicted viewpoint information
		DirectX::XMMATRIX mat_projection = gfx()->GetXRProjection(view.fov.angleLeft, view.fov.angleRight, view.fov.angleUp, view.fov.angleDown, 0.05f, 500.0f);
		DirectX::XMMATRIX mat_view = XMMatrixInverse( nullptr, XMMatrixAffineTransformation(
			DirectX::g_XMOne, DirectX::g_XMZero,
			DirectX::XMLoadFloat4( (DirectX::XMFLOAT4*)&view.pose.orientation ),
			DirectX::XMLoadFloat3( (DirectX::XMFLOAT3*)&view.pose.position ) ) );

		// Set the active shaders and constant buffers.
		gfx()->m_Context->VSSetConstantBuffers( 0, 1, &app_constant_buffer );
		app_vshader->Bind( gfx() );
		app_pshader->Bind( gfx() );

		// Set up the cube mesh's information
		app_vertex_buffer->Bind( gfx() );
		app_index_buffer->Bind( gfx() );
		app_topology->Bind( gfx() );
		app_shader_layout->Bind( gfx() );

		// Put camera matrices into the shader's constant buffer
		app_transform_buffer_t transform_buffer;
		XMStoreFloat4x4( &transform_buffer.viewproj, DirectX::XMMatrixTranspose( mat_view * mat_projection ) );
		uint16_t app_inds[] = {
			1,2,0, 2,3,0, 4,6,5, 7,6,4,
			6,2,1, 5,6,1, 3,7,4, 0,3,4,
			4,5,1, 0,4,1, 2,7,3, 2,6,7, };
		// Draw all the cubes we have in our list!
		for (size_t i = 0; i < app_cubes.size(); i++) {
			// Create a translate, rotate, scale matrix for the cube's world location
			DirectX::XMMATRIX mat_model = XMMatrixAffineTransformation(
				DirectX::g_XMOne * 0.05f, DirectX::g_XMZero,
				DirectX::XMLoadFloat4( (DirectX::XMFLOAT4*)&app_cubes[i].orientation ),
				DirectX::XMLoadFloat3( (DirectX::XMFLOAT3*)&app_cubes[i].position ) );

			// Update the shader's constant buffer with the transform matrix info, and then draw the mesh!
			XMStoreFloat4x4( &transform_buffer.world, DirectX::XMMatrixTranspose( mat_model ) );
			gfx()->m_Context->UpdateSubresource( app_constant_buffer, 0, nullptr, &transform_buffer, 0, 0 );
			gfx()->m_Context->DrawIndexed( (UINT)_countof( app_inds ), 0, 0 );
		}

		XMStoreFloat4x4( &transform_buffer.world, DirectX::XMMatrixTranspose( DirectX::XMMatrixIdentity() ) );
		gfx()->m_Context->UpdateSubresource( app_constant_buffer, 0, nullptr, &transform_buffer, 0, 0 );
		gfx()->m_Context->DrawIndexed( (UINT)_countof( app_inds ), 0, 0 );
		phongVS->Bind( gfx() );
		phongPS->Bind( gfx() );
		m_Model->Render( gfx() );
	}

	void Application::Update()
	{
		// If the user presses the select action, lets add a cube at that location!
		for (uint32_t i = 0; i < 2; i++) {
			if (openxrManager->HandSelect(i)) {
				app_cubes.push_back( openxrManager->GetHandPos( i ) );
				KM_CORE_INFO( "Placing block at x:{}, y:{}, z:{}", openxrManager->GetHandPos( i ).position.x, openxrManager->GetHandPos( i ).position.y, openxrManager->GetHandPos( i ).position.z );
			}
		}
	}

	void Application::Update_Predicted()
	{
		// Update the location of the hand cubes. This is done after the inputs have been updated to 
		// use the predicted location, but during the render code, so we have the most up-to-date location.
		if (app_cubes.size() < 2)
			app_cubes.resize( 2, openxrManager->GetPoseIdentity() );
		for (uint32_t i = 0; i < 2; i++) {
			app_cubes[i] = openxrManager->GetInput().renderHand[i] ? openxrManager->GetInput().handPose[i] : openxrManager->GetPoseIdentity();
		}
	}

}