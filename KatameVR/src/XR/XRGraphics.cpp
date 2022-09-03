#include "XRGraphics.h"

#include "../Core/Log.h"

#pragma comment(lib,"D3D11.lib")
#pragma comment(lib,"D3dcompiler.lib")
#pragma comment(lib,"Dxgi.lib") 

namespace Katame
{
	ID3D11Device* XRGraphics::m_Device = nullptr;
	ID3D11DeviceContext* XRGraphics::m_Context = nullptr;
	int64_t XRGraphics::m_Swapchain_fmt = DXGI_FORMAT_R8G8B8A8_UNORM;
	void* XRGraphics::m_GraphicsBinding = nullptr;
	XrResult XRGraphics::m_LastCallResult = XR_SUCCESS;

	std::vector<XrSwapchainImageD3D11KHR> XRGraphics::m_SwapchainImages_Color_L = {};
	std::vector<XrSwapchainImageD3D11KHR> XRGraphics::m_SwapchainImages_Color_R = {};
	std::vector<XrSwapchainImageD3D11KHR> XRGraphics::m_SwapchainImages_Depth_L = {};
	std::vector<XrSwapchainImageD3D11KHR> XRGraphics::m_SwapchainImages_Depth_R = {};

	void XRGraphics::Init( XrInstance* xrInstance, XrSystemId* xrSystemId, XrSession* xrSession, LUID& adapter_luid )
	{
		KM_CORE_INFO( "Initializing Graphics.." );

		IDXGIAdapter1* adapter = GetAdapter( adapter_luid );
		D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
		if (adapter == nullptr)
			KM_CORE_ERROR( "Failed to get adapter, make sure headset is connected." );

		D3D11CreateDevice( adapter, D3D_DRIVER_TYPE_UNKNOWN, 0, 0, featureLevels, _countof( featureLevels ), D3D11_SDK_VERSION, &m_Device, nullptr, &m_Context );

		adapter->Release();
		KM_CORE_INFO( "Initialized Graphics!" );
	}

	ID3D11Device* XRGraphics::GetDevice()
	{
		return m_Device;
	}

	ID3D11DeviceContext* XRGraphics::GetContext()
	{
		return m_Context;
	}

	XrResult XRGraphics::GenerateSwapchainImages( const XrSwapchain& xrSwapChain, const uint32_t nEye, const bool bIsDepth )
	{
		// Check how many images are in this swapchain from the runtime
		uint32_t nNumOfSwapchainImages;
		XrResult xrResult = xrEnumerateSwapchainImages( xrSwapChain, 0, &nNumOfSwapchainImages, nullptr );
		if (m_LastCallResult != XR_SUCCESS)
			return xrResult;

		// Generate swapchain image holders based on retrieved count from the runtime
		std::vector<XrSwapchainImageD3D11KHR> xrSwapchainImages;
		xrSwapchainImages.resize( nNumOfSwapchainImages );

		for (uint32_t i = 0; i < nNumOfSwapchainImages; i++)
		{
			xrSwapchainImages[i] = { XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR };
		}

		// Retrieve swapchain images from the runtime
		m_LastCallResult = xrEnumerateSwapchainImages(
			xrSwapChain, nNumOfSwapchainImages, &nNumOfSwapchainImages, reinterpret_cast<XrSwapchainImageBaseHeader*>(xrSwapchainImages.data()) );
		if (xrResult != XR_SUCCESS)
			return xrResult;

		// Add the images to our swapchain image cache for drawing to later
		for (uint32_t i = 0; i < (uint32_t)nNumOfSwapchainImages; i++)
		{
			if (nEye == 0)
			{
				if (bIsDepth)
					m_SwapchainImages_Depth_L.push_back( xrSwapchainImages[i] );
				else
					m_SwapchainImages_Color_L.push_back( xrSwapchainImages[i] );
			}
			else
			{
				if (bIsDepth)
					m_SwapchainImages_Depth_R.push_back( xrSwapchainImages[i] );
				else
					m_SwapchainImages_Color_R.push_back( xrSwapchainImages[i] );
			}
		}

		return XR_SUCCESS;
	}

	void* XRGraphics::GetGraphicsBinding()
	{
		return m_GraphicsBinding;
	}

	uint32_t XRGraphics::GetSwapchainImageCount( const EXREye eEye, const bool bIsDepth )
	{
		switch (eEye)
		{
		case EXREye::EYE_LEFT:
			if (bIsDepth)
				return (uint32_t)m_SwapchainImages_Depth_L.size();
			return (uint32_t)m_SwapchainImages_Color_L.size();
			break;

		case EXREye::EYE_RIGHT:
			if (bIsDepth)
				return (uint32_t)m_SwapchainImages_Depth_R.size();
			return (uint32_t)m_SwapchainImages_Color_R.size();
			break;
		}

		return 0;
	}

	std::string XRGraphics::GetTextureFormatName( int64_t nTextureFormat )
	{
		switch (nTextureFormat)
		{
		/*case GL_RGBA16:
			return "GL_RGBA16";

		case GL_RGBA16F:
			return "GL_RGBA16F";

		case GL_RGB16F:
			return "GL_RGB16F";

		case GL_SRGB8:
			return "GL_SRGB8";

		case GL_SRGB8_ALPHA8:
			return "GL_SRGB8_ALPHA8";

		case GL_DEPTH_COMPONENT16:
			return "GL_DEPTH_COMPONENT16";

		case GL_DEPTH_COMPONENT24:
			return "GL_DEPTH_COMPONENT24";*/

		case DXGI_FORMAT_D32_FLOAT:
			return "DXGI_FORMAT_D32_FLOAT";

		default:
			return std::to_string( nTextureFormat );
		}

		return std::to_string( nTextureFormat );
	}

	ID3D11Texture2D* XRGraphics::GetTexture2D( const EXREye eEye, uint32_t nSwapchainIndex, const bool bGetDepth )
	{
		return GetTextures2D( eEye, bGetDepth )[nSwapchainIndex].texture;
	}

	std::vector<XrSwapchainImageD3D11KHR> XRGraphics::GetTextures2D( const EXREye eEye, const bool bGetDepth )
	{
		switch (eEye)
		{
		case EXREye::EYE_LEFT:
			if (bGetDepth)
				return m_SwapchainImages_Depth_L;
			return m_SwapchainImages_Color_L;
			break;

		case EXREye::EYE_RIGHT:
			if (bGetDepth)
				return m_SwapchainImages_Depth_R;
			return m_SwapchainImages_Color_R;
			break;
		}

		return m_SwapchainImages_Color_L;
	}

	bool XRGraphics::IsDepth( int64_t nDepthFormat )
	{
		/*switch (nDepthFormat)
		{
		case GL_DEPTH_COMPONENT16:
		case GL_DEPTH_COMPONENT24:
		case GL_DEPTH_COMPONENT32:
			return true;
		}*/
		return true;

		//return false;
	}

	int64_t XRGraphics::GetDefaultDepthFormat()
	{
		return DXGI_FORMAT_D32_FLOAT;
	}

	IDXGIAdapter1* XRGraphics::GetAdapter( LUID& adapter_luid )
	{
		// Turn the LUID into a specific graphics device adapter
		IDXGIAdapter1* final_adapter = nullptr;
		IDXGIAdapter1* curr_adapter = nullptr;
		IDXGIFactory1* dxgi_factory;
		DXGI_ADAPTER_DESC1 adapter_desc;

		CreateDXGIFactory1( __uuidof(IDXGIFactory1), (void**)(&dxgi_factory) );

		int curr = 0;
		while (dxgi_factory->EnumAdapters1( curr++, &curr_adapter ) == S_OK) {
			curr_adapter->GetDesc1( &adapter_desc );

			if (memcmp( &adapter_desc.AdapterLuid, &adapter_luid, sizeof( &adapter_luid ) ) == 0) {
				final_adapter = curr_adapter;
				break;
			}
			curr_adapter->Release();
			curr_adapter = nullptr;
		}
		dxgi_factory->Release();
		return final_adapter;
	}


}