#include "Graphics.h"
#include "../Core/Log.h"

namespace Katame
{

	Graphics::Graphics( int width, int height )
	{
	}

	Graphics::~Graphics()
	{
		if (m_Context) { m_Context->Release(); m_Context = nullptr; }
		if (m_Device) { m_Device->Release();  m_Device = nullptr; }
	}

	bool Graphics::Init( LUID& adapter_luid )
	{
		KM_CORE_INFO( "Initializing Graphics.." );

		IDXGIAdapter1* adapter = GetAdapter( adapter_luid );
		D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

		if (adapter == nullptr)
		{
			KM_CORE_ERROR( "Failed to get adapter, make sure headset is connected." );
			return false;
		}
		D3D11CreateDevice( adapter, D3D_DRIVER_TYPE_UNKNOWN, 0, 0, featureLevels, _countof( featureLevels ), D3D11_SDK_VERSION, &m_Device, nullptr, &m_Context );

		adapter->Release();

		KM_CORE_INFO( "Initialized Graphics!" );
		return true;
	}

	void Graphics::BeginFrame( float red, float green, float blue ) noexcept
	{
	}

	void Graphics::EndFrame()
	{
	}

	void Graphics::DrawIndexed( int count ) noexcept
	{
		m_Context->DrawIndexed( count, 0u, 0u );
	}

	void Graphics::RenderLayer( float offset_x, float offset_y, float extent_width, float extent_height, swapchain_surfdata_t& surface )
	{
		// Set up where on the render target we want to draw, the view has a 
		D3D11_VIEWPORT viewport = CD3D11_VIEWPORT( offset_x, offset_y, extent_width, extent_height );
		m_Context->RSSetViewports( 1, &viewport );

		// Wipe our swapchain color and depth target clean, and then set them up for rendering!
		float clear[] = { 20.0f / 255.0f, 2.0f / 255.0f, 30.0f / 255.0f, 1 };
		m_Context->ClearRenderTargetView( surface.target_view, clear );
		m_Context->ClearDepthStencilView( surface.depth_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );
		m_Context->OMSetRenderTargets( 1, &surface.target_view, surface.depth_view );
	}

	swapchain_surfdata_t Graphics::MakeSurfaceData( ID3D11Texture2D* texture )
	{
		swapchain_surfdata_t result = {};

		D3D11_TEXTURE2D_DESC      color_desc;
		texture->GetDesc( &color_desc );

		// Create a view resource for the swapchain image target that we can use to set up rendering.
		D3D11_RENDER_TARGET_VIEW_DESC target_desc = {};
		target_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		// NOTE: Why not use color_desc.Format? Check the notes over near the xrCreateSwapchain call!
		// Basically, the color_desc.Format of the OpenXR created swapchain is TYPELESS, but in order to
		// create a View for the texture, we need a concrete variant of the texture format like UNORM.
		target_desc.Format = (DXGI_FORMAT)m_Swapchain_fmt;
		m_Device->CreateRenderTargetView( texture, &target_desc, &result.target_view );

		// Create a depth buffer that matches 
		ID3D11Texture2D* depth_texture;
		D3D11_TEXTURE2D_DESC depth_desc = {};
		depth_desc.SampleDesc.Count = 1;
		depth_desc.MipLevels = 1;
		depth_desc.Width = color_desc.Width;
		depth_desc.Height = color_desc.Height;
		depth_desc.ArraySize = color_desc.ArraySize;
		depth_desc.Format = DXGI_FORMAT_R32_TYPELESS;
		depth_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
		m_Device->CreateTexture2D( &depth_desc, nullptr, &depth_texture );

		// And create a view resource for the depth buffer, so we can set that up for rendering to as well!
		D3D11_DEPTH_STENCIL_VIEW_DESC stencil_desc = {};
		stencil_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		stencil_desc.Format = DXGI_FORMAT_D32_FLOAT;
		m_Device->CreateDepthStencilView( depth_texture, &stencil_desc, &result.depth_view );

		// We don't need direct access to the ID3D11Texture2D object anymore, we only need the view
		depth_texture->Release();

		return result;
	}

	DirectX::XMMATRIX Graphics::GetXRProjection( float angleLeft, float angleRight, float angleUp, float angleDown, float clip_near, float clip_far )
	{
		const float left = clip_near * tanf( angleLeft );
		const float right = clip_near * tanf( angleRight );
		const float down = clip_near * tanf( angleDown );
		const float up = clip_near * tanf( angleUp );

		return DirectX::XMMatrixPerspectiveOffCenterRH( left, right, down, up, clip_near, clip_far );
	}

	IDXGIAdapter1* Graphics::GetAdapter( LUID& adapter_luid )
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