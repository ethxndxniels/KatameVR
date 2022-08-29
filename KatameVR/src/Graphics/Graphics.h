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
		static bool Init( LUID& adapter_luid );
		static void BeginFrame( float red, float green, float blue ) noexcept;
		static void EndFrame();
		static void DrawIndexed( int count )  noexcept;
		static void RenderLayer( float offset_x, float offset_y, float extent_width, float extent_height, swapchain_surfdata_t& surface );
		static swapchain_surfdata_t MakeSurfaceData( ID3D11Texture2D* texture );
		static DirectX::XMMATRIX GetXRProjection( float angleLeft, float angleRight, float angleUp, float angleDown, float clip_near, float clip_far );
	private:
		static IDXGIAdapter1* GetAdapter( LUID& adapter_luid );
	public:
		static ID3D11Device* m_Device;
		static ID3D11DeviceContext* m_Context;
		static int64_t m_Swapchain_fmt;
	};

}