#pragma once

#pragma comment(lib,"D3D11.lib")
#pragma comment(lib,"D3dcompiler.lib") // for shader compile
#pragma comment(lib,"Dxgi.lib") // for CreateDXGIFactory1

#include <d3d11.h>
#include <directxmath.h> // Matrix math functions and objects
#include <d3dcompiler.h> // For compiling shaders! D3DCompile

#include <vector>

namespace Katame
{
	struct swapchain_surfdata_t {
		ID3D11DepthStencilView* depth_view;
		ID3D11RenderTargetView* target_view;
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
		void RenderLayer( struct XrCompositionLayerProjectionView& view, swapchain_surfdata_t& surface );
		swapchain_surfdata_t MakeSurfaceData( struct XrBaseInStructure& swapchain_img );
		void SwapchainDestroy( struct swapchain_t& swapchain );
		DirectX::XMMATRIX GetXRProjection( struct XrFovf fov, float clip_near, float clip_far );
	private:
		IDXGIAdapter1* GetAdapter( LUID& adapter_luid );
	public:
		ID3D11Device* m_Device = nullptr;
		ID3D11DeviceContext* m_Context = nullptr;
		int64_t              m_Swapchain_fmt = DXGI_FORMAT_R8G8B8A8_UNORM;
	};

}