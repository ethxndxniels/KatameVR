#pragma once

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

#include <vector>

namespace Katame
{
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

	struct app_transform_buffer_t {
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4X4 viewproj;
	};

	class Graphics
	{
	public:
		Graphics( int width, int height );
		~Graphics();
	public:
		bool Init( LUID& adapter_luid );
		void BeginFrame( float red, float green, float blue ) noexcept;
		void EndFrame();
		void DrawIndexed( int count )  noexcept;
		void RenderLayer( class XrCompositionLayerProjectionView& view, swapchain_surfdata_t& surface );
		swapchain_surfdata_t MakeSurfaceData( XrBaseInStructure& swapchain_img );
		void SwapchainDestroy( swapchain_t& swapchain );
		DirectX::XMMATRIX GetXRProjection( XrFovf fov, float clip_near, float clip_far );
	private:
		IDXGIAdapter1* GetAdapter( LUID& adapter_luid );
	public:
		ID3D11Device* m_Device = nullptr;
		ID3D11DeviceContext* m_Context = nullptr;
		int64_t              m_Swapchain_fmt = DXGI_FORMAT_R8G8B8A8_UNORM;
	};

}