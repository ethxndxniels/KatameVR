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
		void RenderLayer( float offset_x, float offset_y, float extent_width, float extent_height, swapchain_surfdata_t& surface );
		swapchain_surfdata_t MakeSurfaceData( ID3D11Texture2D* texture );
		DirectX::XMMATRIX GetXRProjection( float angleLeft, float angleRight, float angleUp, float angleDown, float clip_near, float clip_far );
	private:
		IDXGIAdapter1* GetAdapter( LUID& adapter_luid );
	public:
		ID3D11Device* m_Device = nullptr;
		ID3D11DeviceContext* m_Context = nullptr;
		int64_t              m_Swapchain_fmt = DXGI_FORMAT_R8G8B8A8_UNORM;
	};

}