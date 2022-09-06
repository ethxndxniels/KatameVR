#include "Graphics.h"
#include "Common.h"
#include "D3DCommon.h"
#include "../Core/Log.h"

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

		InitializeD3D11DeviceForAdapter( adapter, featureLevels, &m_device, &m_deviceContext );

		InitializeResources();

		m_graphicsBinding.device = m_device;
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

	void Graphics::RenderView( const XrCompositionLayerProjectionView& layerView, const XrSwapchainImageBaseHeader* swapchainImage, int64_t swapchainFormat, const std::vector<Cube>& cubes )
	{
		CHECK( layerView.subImage.imageArrayIndex == 0 );  // Texture arrays not supported.

		ID3D11Texture2D* const colorTexture = reinterpret_cast<const XrSwapchainImageD3D11KHR*>(swapchainImage)->texture;

		CD3D11_VIEWPORT viewport( (float)layerView.subImage.imageRect.offset.x, (float)layerView.subImage.imageRect.offset.y,
			(float)layerView.subImage.imageRect.extent.width,
			(float)layerView.subImage.imageRect.extent.height );
		m_deviceContext->RSSetViewports( 1, &viewport );

		// Create RenderTargetView with original swapchain format (swapchain is typeless).
		ID3D11RenderTargetView* renderTargetView;
		const CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc( D3D11_RTV_DIMENSION_TEXTURE2D, (DXGI_FORMAT)swapchainFormat );
		
		m_device->CreateRenderTargetView( colorTexture, &renderTargetViewDesc, &renderTargetView );

		ID3D11DepthStencilView* depthStencilView = GetDepthStencilView( colorTexture );

		// Clear swapchain and depth buffer. NOTE: This will clear the entire render target view, not just the specified view.
		m_deviceContext->ClearRenderTargetView( renderTargetView, static_cast<const FLOAT*>(m_clearColor.data()) );
		m_deviceContext->ClearDepthStencilView( depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

		ID3D11RenderTargetView* renderTargets[] = { renderTargetView };
		m_deviceContext->OMSetRenderTargets( (UINT)ArraySize( renderTargets ), renderTargets, depthStencilView );

		const DirectX::XMMATRIX spaceToView = XMMatrixInverse( nullptr, LoadXrPose( layerView.pose ) );
		XrMatrix4x4f projectionMatrix;
		XrMatrix4x4f_CreateProjectionFov( &projectionMatrix, GRAPHICS_D3D, layerView.fov, 0.05f, 100.0f );

		// Set shaders and constant buffers.
		ViewProjectionConstantBuffer viewProjection;
		XMStoreFloat4x4( &viewProjection.ViewProjection, XMMatrixTranspose( spaceToView * LoadXrMatrix( projectionMatrix ) ) );
		m_deviceContext->UpdateSubresource( m_viewProjectionCBuffer, 0, nullptr, &viewProjection, 0, 0 );

		ID3D11Buffer* const constantBuffers[] = { m_modelCBuffer, m_viewProjectionCBuffer };
		m_deviceContext->VSSetConstantBuffers( 0, (UINT)ArraySize( constantBuffers ), constantBuffers );
		m_deviceContext->VSSetShader( m_vertexShader, nullptr, 0 );
		m_deviceContext->PSSetShader( m_pixelShader, nullptr, 0 );

		// Set cube primitive data.
		const UINT strides[] = { sizeof( Geometry::Vertex ) };
		const UINT offsets[] = { 0 };
		ID3D11Buffer* vertexBuffers[] = { m_cubeVertexBuffer };
		m_deviceContext->IASetVertexBuffers( 0, (UINT)ArraySize( vertexBuffers ), vertexBuffers, strides, offsets );
		m_deviceContext->IASetIndexBuffer( m_cubeIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );
		m_deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		m_deviceContext->IASetInputLayout( m_inputLayout );

		// Render each cube
		for (const Cube& cube : cubes) {
			// Compute and update the model transform.
			ModelConstantBuffer model;
			XMStoreFloat4x4( &model.Model,
				XMMatrixTranspose( XMMatrixScaling( cube.Scale.x, cube.Scale.y, cube.Scale.z ) * LoadXrPose( cube.Pose ) ) );
			m_deviceContext->UpdateSubresource( m_modelCBuffer, 0, nullptr, &model, 0, 0 );

			// Draw the cube.
			m_deviceContext->DrawIndexed( (UINT)ArraySize( Geometry::c_cubeIndices ), 0, 0 );
		}
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
		CHECK_HRCMD( m_device->CreateVertexShader( vertexShaderBytes, sizeof(vertexShaderBytes), nullptr,
			&m_vertexShader ) );

		const ID3DBlob* pixelShaderBytes = CompileShader( ShaderHlsl, "MainPS", "ps_5_0" );
		CHECK_HRCMD( m_device->CreatePixelShader( pixelShaderBytes, sizeof( pixelShaderBytes), nullptr,
			&m_pixelShader ) );

		const D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		CHECK_HRCMD( m_device->CreateInputLayout( vertexDesc, (UINT)ArraySize( vertexDesc ), vertexShaderBytes,
			sizeof(vertexShaderBytes), &m_inputLayout ) );

		const CD3D11_BUFFER_DESC modelConstantBufferDesc( sizeof( ModelConstantBuffer ), D3D11_BIND_CONSTANT_BUFFER );
		CHECK_HRCMD( m_device->CreateBuffer( &modelConstantBufferDesc, nullptr, &m_modelCBuffer ) );

		const CD3D11_BUFFER_DESC viewProjectionConstantBufferDesc( sizeof( ViewProjectionConstantBuffer ), D3D11_BIND_CONSTANT_BUFFER );
		CHECK_HRCMD(
			m_device->CreateBuffer( &viewProjectionConstantBufferDesc, nullptr, &m_viewProjectionCBuffer ) );

		const D3D11_SUBRESOURCE_DATA vertexBufferData{ Geometry::c_cubeVertices };
		const CD3D11_BUFFER_DESC vertexBufferDesc( sizeof( Geometry::c_cubeVertices ), D3D11_BIND_VERTEX_BUFFER );
		CHECK_HRCMD( m_device->CreateBuffer( &vertexBufferDesc, &vertexBufferData, &m_cubeVertexBuffer ) );

		const D3D11_SUBRESOURCE_DATA indexBufferData{ Geometry::c_cubeIndices };
		const CD3D11_BUFFER_DESC indexBufferDesc( sizeof( Geometry::c_cubeIndices ), D3D11_BIND_INDEX_BUFFER );
		CHECK_HRCMD( m_device->CreateBuffer( &indexBufferDesc, &indexBufferData, &m_cubeIndexBuffer ) );
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