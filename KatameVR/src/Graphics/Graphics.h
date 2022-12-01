#pragma once

#pragma comment(lib,"D3D11.lib")
#pragma comment(lib,"D3dcompiler.lib") // for shader compile
#pragma comment(lib,"Dxgi.lib") // for CreateDXGIFactory1

#include <d3d11.h>
#include <directxmath.h> // Matrix math functions and objects
#include <d3dcompiler.h> // For compiling shaders! D3DCompile

#include <vector>
#include <array>

// Tell OpenXR what platform code we'll be using
#define XR_USE_PLATFORM_WIN32
#define XR_USE_GRAPHICS_API_D3D11
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <openxr/openxr_platform_defines.h>
#include <openxr/openxr_reflection.h>

#include <map>
#include <list>

#include "D3DCommon.h"
#include "../Geometry/Geometry.h"

// Bindables
#include "../Bindables/VertexShader.h"
#include "../Bindables/PixelShader.h"
#include "../Bindables/InputLayout.h"
#include "../Bindables/CBuffer.h"
#include "../Bindables/Buffer.h"

namespace Katame
{
	class Renderer;

	class Graphics
	{
	public:
		Graphics() = default;
		~Graphics();
	public:
		void InitializeDevice( XrInstance instance, XrSystemId systemId );
		std::vector<XrSwapchainImageBaseHeader*> AllocateSwapchainImageStructs( uint32_t capacity, const XrSwapchainCreateInfo& swapchainCreateInfo );
		void RenderView( const XrCompositionLayerProjectionView& layerView, const XrSwapchainImageBaseHeader* swapchainImage, int64_t swapchainFormat );
		int64_t SelectColorSwapchainFormat( const std::vector<int64_t>& runtimeFormats ) const;
		void InitializeRenderer( Renderer* renderer );
		void DrawIndexed( UINT indexCount, UINT startIndexLocation, UINT baseVertexLocation );
	private:
		void InitializeD3D11DeviceForAdapter( IDXGIAdapter1* adapter, const std::vector<D3D_FEATURE_LEVEL>& featureLevels,
			ID3D11Device** device, ID3D11DeviceContext** deviceContext );
		void InitializeResources();
		IDXGIAdapter1* GetAdapter( LUID adapterId );
		ID3D11DepthStencilView* GetDepthStencilView( ID3D11Texture2D* colorTexture );
	public:
		ID3D11Device* m_Device;
		ID3D11DeviceContext* m_Context;
		Renderer* m_Renderer;
	private:
		XrGraphicsBindingD3D11KHR m_graphicsBinding{ XR_TYPE_GRAPHICS_BINDING_D3D11_KHR };
		std::list<std::vector<XrSwapchainImageD3D11KHR>> m_swapchainImageBuffers;

		VCBuffer* m_ModelCBuf;
		VCBuffer* m_ViewProjCBuf;

		// Map color buffer to associated depth buffer. This map is populated on demand.
		std::map<ID3D11Texture2D*, ID3D11DepthStencilView*> m_colorToDepthMap;
		std::array<float, 4> m_clearColor;
	};

}