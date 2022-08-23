#pragma once

// Tell OpenXR what platform code we'll be using
#define XR_USE_PLATFORM_WIN32
#define XR_USE_GRAPHICS_API_D3D11

#include <d3d11.h>
#include <directxmath.h> // Matrix math functions and objects
#include <d3dcompiler.h> // For compiling shaders! D3DCompile
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include "../Graphics/Graphics.h"

namespace Katame
{
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


	class OpenXRManager
	{
	public:
		OpenXRManager();
		~OpenXRManager();
	public:
		Graphics* gfx = nullptr;
	public:
		bool HandSelect(int i);
		XrPosef GetHandPos( int i );
		const XrPosef GetPoseIdentity();
		input_state_t GetInput();
		void SwapchainDestroy( swapchain_t& swapchain );

		bool openxr_init( const char* app_name, int64_t swapchain_format );
		void openxr_poll_events( bool& m_Running, bool& xr_running );
		void openxr_make_actions();
		bool openxr_render_layer( XrTime predictedTime, std::vector<XrCompositionLayerProjectionView>& views, XrCompositionLayerProjection& layer, class Application* app );
		void openxr_poll_actions();
		void openxr_render_frame( class Application* app );
		void openxr_shutdown();
		void openxr_poll_predicted( XrTime predicted_time );
		XrSessionState get_session_state() { return xr_session_state; };
	private:
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
	};

}