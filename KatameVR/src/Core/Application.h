#pragma once

#include <string>

#include "Graphics.h"

#include <thread> // sleep_for
#include <vector>
#include <algorithm> // any_of


#include "IndexedTriangleList.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "InputLayout.h"

namespace Katame 
{

	class Application
	{
	public:
		Application();
		~Application();

		void Launch();

	private:
		Graphics* gfx;
	public:

		void Draw( XrCompositionLayerProjectionView& view );
		void Update();
		void Update_Predicted();

		// OPENXR
		bool openxr_init( const char* app_name, int64_t swapchain_format );
		void openxr_poll_events();
		void openxr_make_actions();
		bool openxr_render_layer( XrTime predictedTime, std::vector<XrCompositionLayerProjectionView>& views, XrCompositionLayerProjection& layer );
		void openxr_poll_actions();
		void openxr_render_frame();
		void openxr_shutdown();
		void openxr_poll_predicted( XrTime predicted_time );
		XrSessionState get_session_state() { return xr_session_state; };
	private:
		bool m_Running = true, m_Minimized = false;
		bool xr_running = false;

		XrSessionState xr_session_state = XR_SESSION_STATE_UNKNOWN;

		const XrPosef  xr_pose_identity = { {0,0,0,1}, {0,0,0} };
		XrInstance     xr_instance = {};
		XrSession      xr_session = {};


		XrSpace        xr_app_space = {};
		XrSystemId     xr_system_id = XR_NULL_SYSTEM_ID;
		input_state_t  xr_input = { };
		XrEnvironmentBlendMode   xr_blend = {};
		XrDebugUtilsMessengerEXT xr_debug = {};

		std::vector<XrView>                  xr_views;
		std::vector<XrViewConfigurationView> xr_config_views;
		std::vector<swapchain_t>             xr_swapchains;
		XrFormFactor            app_config_form = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
		XrViewConfigurationType app_config_view = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

		// Function pointers for some OpenXR extension methods we'll use.
		PFN_xrGetD3D11GraphicsRequirementsKHR ext_xrGetD3D11GraphicsRequirementsKHR = nullptr;
		PFN_xrCreateDebugUtilsMessengerEXT    ext_xrCreateDebugUtilsMessengerEXT = nullptr;
		PFN_xrDestroyDebugUtilsMessengerEXT   ext_xrDestroyDebugUtilsMessengerEXT = nullptr;

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
