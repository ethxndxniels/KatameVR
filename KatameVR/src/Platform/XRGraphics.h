#pragma once

#include "XRCommon.h"

#include <d3d11.h>

namespace Katame
{
	class XRGraphics
	{
	public:
		static void Init( XrInstance* xrInstance, XrSystemId* xrSystemId, XrSession* xrSession, XrResult* xrResult );
		static ID3D11Device* GetDevice();
		static ID3D11DeviceContext* GetContext();
		static XrResult GenerateSwapchainImages( const XrSwapchain& xrSwapChain, const uint32_t nEye, const bool bIsDepth );
		static void* GetGraphicsBinding();
		static uint32_t GetSwapchainImageCount( const EXREye eEye, const bool bIsDepth = false );
		static std::string GetTextureFormatName( int64_t nTextureFormat );
		static ID3D11Texture2D* GetTexture2D( const EXREye eEye, uint32_t nSwapchainIndex, const bool bGetDepth = false );
		static std::vector< XrSwapchainImageD3D11KHR > GetTextures2D( const EXREye eEye, const bool bGetDepth = false );
		static bool IsDepth( int64_t nDepthFormat );
		static int64_t GetDefaultDepthFormat();
	private:
		static IDXGIAdapter1* GetAdapter( LUID& adapter_luid );
	private:
		static ID3D11Device* m_Device;
		static ID3D11DeviceContext* m_Context;
		static int64_t m_Swapchain_fmt;
		static void* m_GraphicsBinding;
	private:
		XrSession* m_xrSession = XR_NULL_HANDLE;
		static std::vector<XrSwapchainImageD3D11KHR> m_SwapchainImages_Color_L;
		static std::vector<XrSwapchainImageD3D11KHR> m_SwapchainImages_Color_R;
		static std::vector<XrSwapchainImageD3D11KHR> m_SwapchainImages_Depth_L;
		static std::vector<XrSwapchainImageD3D11KHR> m_SwapchainImages_Depth_R;
	};
}

