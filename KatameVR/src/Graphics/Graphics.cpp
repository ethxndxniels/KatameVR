#include "Graphics.h"
#include "Common.h"
#include "D3DCommon.h"
#include "../Core/Log.h"

#include "../Utilities/xr_linear.h"

#include "../Renderer/Renderer.h"

namespace Katame
{
	void Graphics::InitializeDevice( XrInstance instance, XrSystemId systemId )
	{
		m_clearColor = { 20.0f / 255.0f, 2.0f / 255.0f, 30.0f / 255.0f, 1 };

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

		InitializeD3D11DeviceForAdapter( adapter, featureLevels, &m_Device, &m_Context );

		InitializeResources();

		m_graphicsBinding.device = m_Device;
	}

	std::vector<XrSwapchainImageBaseHeader*> Graphics::AllocateSwapchainImageStructs( uint32_t capacity, const XrSwapchainCreateInfo& swapchainCreateInfo )
	{
		// Allocate and initialize the buffer of image structs (must be sequential in memory for xrEnumerateSwapchainImages).
		// Return back an array of pointers to each swapchain image struct so the consumer doesn't need to know the type/size.
		std::vector<XrSwapchainImageD3D11KHR> swapchainImageBuffer( capacity );
		std::vector<XrSwapchainImageBaseHeader*> swapchainImageBase;
		for (XrSwapchainImageD3D11KHR& image : swapchainImageBuffer) 
		{
			image.type = XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR;
			swapchainImageBase.push_back( reinterpret_cast<XrSwapchainImageBaseHeader*>(&image) );
		}

		// Keep the buffer alive by moving it into the list of buffers.
		m_swapchainImageBuffers.push_back( std::move( swapchainImageBuffer ) );

		return swapchainImageBase;
	}

	void Graphics::RenderView( const XrCompositionLayerProjectionView& layerView, const XrSwapchainImageBaseHeader* swapchainImage, int64_t swapchainFormat, const std::vector<CubeData>& cubes )
	{
		CHECK( layerView.subImage.imageArrayIndex == 0 );  // Texture arrays not supported.

		ID3D11Texture2D* const colorTexture = reinterpret_cast<const XrSwapchainImageD3D11KHR*>(swapchainImage)->texture;

		CD3D11_VIEWPORT viewport( (float)layerView.subImage.imageRect.offset.x, (float)layerView.subImage.imageRect.offset.y,
			(float)layerView.subImage.imageRect.extent.width,
			(float)layerView.subImage.imageRect.extent.height );
		m_Context->RSSetViewports( 1, &viewport );

		// Create RenderTargetView with original swapchain format (swapchain is typeless).
		ID3D11RenderTargetView* renderTargetView;
		const CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc( D3D11_RTV_DIMENSION_TEXTURE2D, (DXGI_FORMAT)swapchainFormat );
		
		m_Device->CreateRenderTargetView( colorTexture, &renderTargetViewDesc, &renderTargetView );

		ID3D11DepthStencilView* depthStencilView = GetDepthStencilView( colorTexture );

		// Clear swapchain and depth buffer. NOTE: This will clear the entire render target view, not just the specified view.
		m_Context->ClearRenderTargetView( renderTargetView, static_cast<const FLOAT*>(m_clearColor.data()) );
		m_Context->ClearDepthStencilView( depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

		ID3D11RenderTargetView* renderTargets[] = { renderTargetView };
		m_Context->OMSetRenderTargets( (UINT)ArraySize( renderTargets ), renderTargets, depthStencilView );

		const DirectX::XMMATRIX spaceToView = XMMatrixInverse( nullptr, LoadXrPose( layerView.pose ) );
		XrMatrix4x4f projectionMatrix;
		XrMatrix4x4f_CreateProjectionFov( &projectionMatrix, GRAPHICS_D3D, layerView.fov, 0.05f, 100.0f );

		// Set shaders and constant buffers.
		ViewProjectionConstantBuffer viewProjection;
		XMStoreFloat4x4( &viewProjection.ViewProjection, DirectX::XMMatrixTranspose( spaceToView * LoadXrMatrix( projectionMatrix ) ) );
		m_ViewProjCBuf->Update( this, &viewProjection );
		m_ViewProjCBuf->Bind( this );

		// Render each cube
		for (const CubeData& cube : cubes)
		{
			// Compute and update the model transform.
			ModelConstantBuffer model;
			XMStoreFloat4x4( &model.Model,
				XMMatrixTranspose( DirectX::XMMatrixScaling( cube.Scale.x, cube.Scale.y, cube.Scale.z ) * LoadXrPose( cube.Pose ) ) );
			m_ModelCBuf->Update( this, &model );
			m_ModelCBuf->Bind( this );

			// Draw the cube.
			DrawIndexed( Geometry::CreateCubeIndices().size(), 0, 0 );
		}

		//m_Renderer->Execute();
	}

	int64_t Graphics::SelectColorSwapchainFormat( const std::vector<int64_t>& runtimeFormats ) const
	{
		// List of supported color swapchain formats.
		constexpr DXGI_FORMAT SupportedColorSwapchainFormats[] = 
		{
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT_B8G8R8A8_UNORM,
			DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
			DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
		};

		auto swapchainFormatIt =
			std::find_first_of( runtimeFormats.begin(), runtimeFormats.end(), std::begin( SupportedColorSwapchainFormats ),
				std::end( SupportedColorSwapchainFormats ) );
		if (swapchainFormatIt == runtimeFormats.end()) 
		{
			THROW( "No runtime swapchain format supported for color swapchain" );
		}

		return *swapchainFormatIt;
	}

	void Graphics::InitializeRenderer( Renderer* renderer )
	{
		m_Renderer = renderer;
	}

	void Graphics::DrawIndexed( UINT indexCount, UINT startIndexLocation, UINT baseVertexLocation )
	{
		m_Context->DrawIndexed( indexCount, startIndexLocation, baseVertexLocation );
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
		if (FAILED( hr )) 
		{
			// If initialization failed, it may be because device debugging isn't supported, so retry without that.
			if ((creationFlags & D3D11_CREATE_DEVICE_DEBUG) && (hr == DXGI_ERROR_SDK_COMPONENT_MISSING)) 
			{
				creationFlags &= ~D3D11_CREATE_DEVICE_DEBUG;
				goto TryAgain;
			}

			// If the initialization still fails, fall back to the WARP device.
			// For more information on WARP, see: http://go.microsoft.com/fwlink/?LinkId=286690
			if (driverType != D3D_DRIVER_TYPE_WARP) 
			{
				driverType = D3D_DRIVER_TYPE_WARP;
				goto TryAgain;
			}
		}
	}

	void Graphics::InitializeResources()
	{
		const CD3D11_BUFFER_DESC modelConstantBufferDesc( sizeof( ModelConstantBuffer ), D3D11_BIND_CONSTANT_BUFFER );
		m_ModelCBuf = new VCBuffer( this, modelConstantBufferDesc, 0u );

		const CD3D11_BUFFER_DESC viewProjectionConstantBufferDesc( sizeof( ViewProjectionConstantBuffer ), D3D11_BIND_CONSTANT_BUFFER );
		m_ViewProjCBuf = new VCBuffer( this, viewProjectionConstantBufferDesc, 1u );
	}

	IDXGIAdapter1* Katame::Graphics::GetAdapter( LUID adapterId )
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

			if (memcmp( &adapter_desc.AdapterLuid, &adapterId, sizeof( &adapterId ) ) == 0) {
				final_adapter = curr_adapter;
				break;
			}
			curr_adapter->Release();
			curr_adapter = nullptr;
		}
		dxgi_factory->Release();
		return final_adapter;
	}

	ID3D11DepthStencilView* Graphics::GetDepthStencilView( ID3D11Texture2D* colorTexture )
	{
		// If a depth-stencil view has already been created for this back-buffer, use it.
		auto depthBufferIt = m_colorToDepthMap.find( colorTexture );
		if (depthBufferIt != m_colorToDepthMap.end())
		{
			return depthBufferIt->second;
		}

		// This back-buffer has no corresponding depth-stencil texture, so create one with matching dimensions.
		D3D11_TEXTURE2D_DESC colorDesc;
		colorTexture->GetDesc( &colorDesc );

		D3D11_TEXTURE2D_DESC depthDesc{};
		depthDesc.Width = colorDesc.Width;
		depthDesc.Height = colorDesc.Height;
		depthDesc.ArraySize = colorDesc.ArraySize;
		depthDesc.MipLevels = 1;
		depthDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		depthDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
		depthDesc.SampleDesc.Count = 1;
		ID3D11Texture2D* depthTexture;
		m_Device->CreateTexture2D( &depthDesc, nullptr, &depthTexture );

		// Create and cache the depth stencil view.
		ID3D11DepthStencilView* depthStencilView;
		CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc( D3D11_DSV_DIMENSION_TEXTURE2D, DXGI_FORMAT_D32_FLOAT );
		m_Device->CreateDepthStencilView( depthTexture, &depthStencilViewDesc, &depthStencilView );
		depthBufferIt = m_colorToDepthMap.insert( std::make_pair( colorTexture, depthStencilView ) ).first;

		return depthStencilView;
	}


}