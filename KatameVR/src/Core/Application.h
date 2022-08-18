#pragma once

#include <string>

#pragma comment(lib,"D3D11.lib")
#pragma comment(lib,"D3dcompiler.lib") // for shader compile
#pragma comment(lib,"Dxgi.lib") // for CreateDXGIFactory1

// Tell OpenXR what platform code we'll be using
#define XR_USE_PLATFORM_WIN32
#define XR_USE_GRAPHICS_API_D3D11

#include <d3d11.h>
#include <directxmath.h> // Matrix math functions and objects
#include <d3dcompiler.h> // For compiling shaders! D3DCompile
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <thread> // sleep_for
#include <vector>
#include <algorithm> // any_of

namespace Katame {

	float app_verts[] = {
-1,-1,-1, -1,-1,-1, // Bottom verts
 1,-1,-1,  1,-1,-1,
 1, 1,-1,  1, 1,-1,
-1, 1,-1, -1, 1,-1,
-1,-1, 1, -1,-1, 1, // Top verts
 1,-1, 1,  1,-1, 1,
 1, 1, 1,  1, 1, 1,
-1, 1, 1, -1, 1, 1, };

	uint16_t app_inds[] = {
		1,2,0, 2,3,0, 4,6,5, 7,6,4,
		6,2,1, 5,6,1, 3,7,4, 0,3,4,
		4,5,1, 0,4,1, 2,7,3, 2,6,7, };

	struct swapchain_surfdata_t {
		ID3D11DepthStencilView* depth_view;
		ID3D11RenderTargetView* target_view;
	};

	struct swapchain_t {
		XrSwapchain handle;
		int32_t     width;
		int32_t     height;
		std::vector<XrSwapchainImageD3D11KHR> surface_images;
		std::vector<swapchain_surfdata_t>     surface_data;
	};

	struct input_state_t {
		XrActionSet actionSet;
		XrAction    poseAction;
		XrAction    selectAction;
		XrPath   handSubactionPath[2];
		XrSpace  handSpace[2];
		XrPosef  handPose[2];
		XrBool32 renderHand[2];
		XrBool32 handSelect[2];
	};


	ID3D11Device* d3d_device = nullptr;
	ID3D11DeviceContext* d3d_context = nullptr;
	int64_t              d3d_swapchain_fmt = DXGI_FORMAT_R8G8B8A8_UNORM;

	ID3D11VertexShader* app_vshader;
	ID3D11PixelShader* app_pshader;
	ID3D11InputLayout* app_shader_layout;
	ID3D11Buffer* app_constant_buffer;
	ID3D11Buffer* app_vertex_buffer;
	ID3D11Buffer* app_index_buffer;

	struct app_transform_buffer_t {
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4X4 viewproj;
	};
	constexpr char app_shader_code[] = R"_(
	cbuffer TransformBuffer : register(b0) {
		float4x4 world;
		float4x4 viewproj;
	};
	struct vsIn {
		float4 pos  : SV_POSITION;
		float3 norm : NORMAL;
	};
	struct psIn {
		float4 pos   : SV_POSITION;
		float3 color : COLOR0;
	};
	psIn vs(vsIn input) {
		psIn output;
		output.pos = mul(float4(input.pos.xyz, 1), world);
		output.pos = mul(output.pos, viewproj);
		float3 normal = normalize(mul(float4(input.norm, 0), world).xyz);
		output.color = saturate(dot(normal, float3(0,1,0))).xxx;
		return output;
	}
	float4 ps(psIn input) : SV_TARGET {
		return float4(input.color, 1);
	})_";

	class Application
	{
	public:
		Application();
		~Application();

		void Launch();

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
		void d3d_shutdown();
		bool d3d_init( LUID & adapter_luid );
		void d3d_render_layer( XrCompositionLayerProjectionView& view, struct swapchain_surfdata_t& surface );
		DirectX::XMMATRIX d3d_xr_projection( XrFovf fov, float clip_near, float clip_far );
		IDXGIAdapter1* d3d_get_adapter( LUID& adapter_luid );
		void openxr_poll_predicted( XrTime predicted_time );
		XrSessionState get_session_state() { return xr_session_state; };
		// 

		//virtual void OnEvent( Event& event );

		//void PushLayer( Layer* layer );
		//void PushOverlay( Layer* layer );
		//void RenderImGui();

		//std::string OpenFile( const std::string& filter ) const;
	private:
		//bool OnWindowResize( WindowResizeEvent& e );
		//bool OnWindowClose( WindowCloseEvent& e );
	private:
		//std::unique_ptr<Window> m_Window;
		bool m_Running = true, m_Minimized = false;
		//LayerStack m_LayerStack;
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

	};

}
