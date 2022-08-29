#include "XRGraphics.h"

#include "../Core/Log.h"

namespace Katame
{
	ID3D11Device* XRGraphics::m_Device = nullptr;
	ID3D11DeviceContext* XRGraphics::m_Context = nullptr;
	int64_t XRGraphics::m_Swapchain_fmt = DXGI_FORMAT_R8G8B8A8_UNORM;
	void* XRGraphics::m_GraphicsBinding = nullptr;

	void XRGraphics::Init( XrInstance* xrInstance, XrSystemId* xrSystemId, XrSession* xrSession, XrResult* xrResult )
	{
		KM_CORE_INFO( "Initializing Graphics.." );

		XrGraphicsRequirementsD3D11KHR xrRequirements = { XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR };
		PFN_xrGetD3D11GraphicsRequirementsKHR xrGetOpenGLGraphicsRequirementsKHR = nullptr;
		xrGetInstanceProcAddr( *xrInstance, "xrGetOpenGLGraphicsRequirementsKHR", (PFN_xrVoidFunction*)&xrGetOpenGLGraphicsRequirementsKHR );

		IDXGIAdapter1* adapter = GetAdapter( xrRequirements.adapterLuid );
		D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

		if (adapter == nullptr)
			KM_CORE_ERROR( "Failed to get adapter, make sure headset is connected." );

		D3D11CreateDevice( adapter, D3D_DRIVER_TYPE_UNKNOWN, 0, 0, featureLevels, _countof( featureLevels ), D3D11_SDK_VERSION, &m_Device, nullptr, &m_Context );

		adapter->Release();

		// Setup Graphics bindings
		XrGraphicsBindingD3D11KHR xrGraphicsBinding = { XR_TYPE_GRAPHICS_BINDING_D3D11_KHR };
		xrGraphicsBinding.device = m_Device;

		*xrResult = xrGetOpenGLGraphicsRequirementsKHR( *xrInstance, *xrSystemId, &xrRequirements );

		if (*xrResult != XR_SUCCESS)
			return; // Not throwing a runtime error here as we want to report why the create session failed.

		// Create Session
		XrSessionCreateInfo xrSessionCreateInfo = { XR_TYPE_SESSION_CREATE_INFO };
		xrSessionCreateInfo.next = &xrGraphicsBinding;
		xrSessionCreateInfo.systemId = *xrSystemId;

		*xrResult = xrCreateSession( *xrInstance, &xrSessionCreateInfo, xrSession );

		if (*xrResult != XR_SUCCESS)
			return; // Not throwing a runtime error here as we want to report why the create session failed.

		m_GraphicsBinding = &xrGraphicsBinding;
		return;
	}

	void* XRGraphics::GetGraphicsBinding()
	{
		return m_GraphicsBinding;
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