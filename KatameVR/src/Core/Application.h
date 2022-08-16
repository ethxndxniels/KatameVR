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

	class Application
	{
	public:
		Application();
		~Application();

		void Run();

		void Draw( XrCompositionLayerProjectionView& view );
		void Update();
		void Update_Predicted();

		// OPENXR
		bool openxr_init( const char* app_name, int64_t swapchain_format );
		void openxr_poll_events( bool& exit );
		void openxr_make_actions();
		bool openxr_render_layer( XrTime predictedTime, std::vector<XrCompositionLayerProjectionView>& views, XrCompositionLayerProjection& layer );
		void openxr_poll_actions();
		void openxr_render_frame();
		bool get_xr_running() { return xr_running; };
		void openxr_shutdown();
		void d3d_shutdown();
		bool d3d_init( LUID & adapter_luid );
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
		//ImGuiLayer* m_ImGuiLayer;
		bool xr_running = false;
		XrSessionState xr_session_state = XR_SESSION_STATE_UNKNOWN;

	};

}
