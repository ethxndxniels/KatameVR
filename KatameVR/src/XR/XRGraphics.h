#pragma once

#include "XRCommon.h"

namespace Katame
{
	class XRGraphics
	{
	public:
		static void Init( XrInstance* xrInstance, XrSystemId* xrSystemId, XrSession* xrSession );
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
	public:
		static void ClearRenderTargetView( ID3D11RenderTargetView* rtv, float clear[] );
		static void RSSetViewports( UINT numViews, D3D11_VIEWPORT& viewport );
		static void ClearDepthStencilView( ID3D11DepthStencilView* dsv, UINT clearFlags, float depth, UINT stencil );
		static void OMSetRenderTargets( UINT numViews, ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv );
	private:
		static IDXGIAdapter1* GetAdapter( LUID& adapter_luid );
	private:
		static ID3D11Device* m_Device;
		static ID3D11DeviceContext* m_Context;
		static int64_t m_Swapchain_fmt;
		static void* m_GraphicsBinding;
		static XrResult m_LastCallResult;
	private:
		static std::vector<XrSwapchainImageD3D11KHR> m_SwapchainImages_Color_L;
		static std::vector<XrSwapchainImageD3D11KHR> m_SwapchainImages_Color_R;
		static std::vector<XrSwapchainImageD3D11KHR> m_SwapchainImages_Depth_L;
		static std::vector<XrSwapchainImageD3D11KHR> m_SwapchainImages_Depth_R;
	private:
		static PFN_xrGetD3D11GraphicsRequirementsKHR xrGetD3D11GraphicsRequirementsKHR;
		static PFN_xrCreateDebugUtilsMessengerEXT    xrCreateDebugUtilsMessengerEXT;
		static PFN_xrDestroyDebugUtilsMessengerEXT   xrDestroyDebugUtilsMessengerEXT;
		static XrDebugUtilsMessengerEXT xr_debug;
		static XrEnvironmentBlendMode   xr_blend;
		static const XrPosef  xr_pose_identity;
	};
}

