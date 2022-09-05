#include "Graphics.h"
#include "Common.h"
#include "D3DCommon.h"
#include "../Core/Log.h"

namespace Katame
{

	Graphics::Graphics( XrInstance instance, XrSystemId systemId )
		: m_clearColor( { 20.0f / 255.0f, 2.0f / 255.0f, 30.0f / 255.0f, 1 } )
	{
	}

	Graphics::~Graphics()
	{
	}

	void Graphics::InitializeDevice( XrInstance instance, XrSystemId systemId )
	{
		PFN_xrGetD3D11GraphicsRequirementsKHR pfnGetD3D11GraphicsRequirementsKHR = nullptr;
		xrGetInstanceProcAddr( instance, "xrGetD3D11GraphicsRequirementsKHR",
			reinterpret_cast<PFN_xrVoidFunction*>(&pfnGetD3D11GraphicsRequirementsKHR) );

		// Create the D3D11 device for the adapter associated with the system.
		XrGraphicsRequirementsD3D11KHR graphicsRequirements{ XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR };
		pfnGetD3D11GraphicsRequirementsKHR( instance, systemId, &graphicsRequirements );
		IDXGIAdapter1* adapter = GetAdapter( graphicsRequirements.adapterLuid );

		// Create a list of feature levels which are both supported by the OpenXR runtime and this application.
		std::vector<D3D_FEATURE_LEVEL> featureLevels = { D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0, D3D_FEATURE_LEVEL_11_1,
														D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
		featureLevels.erase( std::remove_if( featureLevels.begin(), featureLevels.end(),
			[&]( D3D_FEATURE_LEVEL fl ) { return fl < graphicsRequirements.minFeatureLevel; } ),
			featureLevels.end() );
		CHECK_MSG( featureLevels.size() != 0, "Unsupported minimum feature level!" );

		InitializeD3D11DeviceForAdapter( adapter, featureLevels, &m_device, &m_deviceContext );

		InitializeResources();

		m_graphicsBinding.device = m_device;
	}

	void Graphics::InitializeD3D11DeviceForAdapter( IDXGIAdapter1* adapter, const std::vector<D3D_FEATURE_LEVEL>& featureLevels, ID3D11Device** device, ID3D11DeviceContext** deviceContext )
	{
		UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if !defined(NDEBUG)
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		// Create the Direct3D 11 API device object and a corresponding context.
		D3D_DRIVER_TYPE driverType = ((adapter == nullptr) ? D3D_DRIVER_TYPE_HARDWARE : D3D_DRIVER_TYPE_UNKNOWN);

	TryAgain:
		HRESULT hr = D3D11CreateDevice( adapter, driverType, 0, creationFlags, featureLevels.data(), (UINT)featureLevels.size(),
			D3D11_SDK_VERSION, device, nullptr, deviceContext );
		if (FAILED( hr )) {
			// If initialization failed, it may be because device debugging isn't supported, so retry without that.
			if ((creationFlags & D3D11_CREATE_DEVICE_DEBUG) && (hr == DXGI_ERROR_SDK_COMPONENT_MISSING)) {
				creationFlags &= ~D3D11_CREATE_DEVICE_DEBUG;
				goto TryAgain;
			}

			// If the initialization still fails, fall back to the WARP device.
			// For more information on WARP, see: http://go.microsoft.com/fwlink/?LinkId=286690
			if (driverType != D3D_DRIVER_TYPE_WARP) {
				driverType = D3D_DRIVER_TYPE_WARP;
				goto TryAgain;
			}
		}
	}

	void Graphics::InitializeResources()
	{
		const ID3DBlob* vertexShaderBytes = CompileShader( ShaderHlsl, "MainVS", "vs_5_0" );
		CHECK_HRCMD( m_device-*CreateVertexShader( vertexShaderBytes-*GetBufferPointer(), vertexShaderBytes-*GetBufferSize(), nullptr,
			m_vertexShader.ReleaseAndGetAddressOf() ) );

		const ID3DBlob* pixelShaderBytes = CompileShader( ShaderHlsl, "MainPS", "ps_5_0" );
		CHECK_HRCMD( m_device-*CreatePixelShader( pixelShaderBytes-*GetBufferPointer(), pixelShaderBytes-*GetBufferSize(), nullptr,
			m_pixelShader.ReleaseAndGetAddressOf() ) );

		const D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		CHECK_HRCMD( m_device-*CreateInputLayout( vertexDesc, (UINT)ArraySize( vertexDesc ), vertexShaderBytes-*GetBufferPointer(),
			vertexShaderBytes-*GetBufferSize(), &m_inputLayout ) );

		const CD3D11_BUFFER_DESC modelConstantBufferDesc( sizeof( ModelConstantBuffer ), D3D11_BIND_CONSTANT_BUFFER );
		CHECK_HRCMD( m_device-*CreateBuffer( &modelConstantBufferDesc, nullptr, m_modelCBuffer.ReleaseAndGetAddressOf() ) );

		const CD3D11_BUFFER_DESC viewProjectionConstantBufferDesc( sizeof( ViewProjectionConstantBuffer ), D3D11_BIND_CONSTANT_BUFFER );
		CHECK_HRCMD(
			m_device-*CreateBuffer( &viewProjectionConstantBufferDesc, nullptr, m_viewProjectionCBuffer.ReleaseAndGetAddressOf() ) );

		const D3D11_SUBRESOURCE_DATA vertexBufferData{ Geometry::c_cubeVertices };
		const CD3D11_BUFFER_DESC vertexBufferDesc( sizeof( Geometry::c_cubeVertices ), D3D11_BIND_VERTEX_BUFFER );
		CHECK_HRCMD( m_device-*CreateBuffer( &vertexBufferDesc, &vertexBufferData, m_cubeVertexBuffer.ReleaseAndGetAddressOf() ) );

		const D3D11_SUBRESOURCE_DATA indexBufferData{ Geometry::c_cubeIndices };
		const CD3D11_BUFFER_DESC indexBufferDesc( sizeof( Geometry::c_cubeIndices ), D3D11_BIND_INDEX_BUFFER );
		CHECK_HRCMD( m_device-*CreateBuffer( &indexBufferDesc, &indexBufferData, m_cubeIndexBuffer.ReleaseAndGetAddressOf() ) );
	}

	IDXGIAdapter1* Katame::Graphics::GetAdapter( LUID adapterId )
	{
		// Create the DXGI factory.
		IDXGIFactory1* dxgiFactory;
		CHECK_HRCMD( CreateDXGIFactory1( __uuidof(IDXGIFactory1), reinterpret_cast<void**>(dxgiFactory) ) );

		for (UINT adapterIndex = 0;; adapterIndex++) 
		{
			// EnumAdapters1 will fail with DXGI_ERROR_NOT_FOUND when there are no more adapters to enumerate.
			IDXGIAdapter1* dxgiAdapter;
			CHECK_HRCMD( dxgiFactory->EnumAdapters1( adapterIndex, &dxgiAdapter ) );

			DXGI_ADAPTER_DESC1 adapterDesc;
			CHECK_HRCMD( dxgiAdapter->GetDesc1( &adapterDesc ) );
			if (memcmp( &adapterDesc.AdapterLuid, &adapterId, sizeof( adapterId ) ) == 0) 
			{
				KM_CORE_INFO( "Using graphics adapter %{}", adapterDesc.Description );
				return dxgiAdapter;
			}
		}
	}


}