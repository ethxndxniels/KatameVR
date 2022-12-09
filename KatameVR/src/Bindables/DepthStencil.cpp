#include "DepthStencil.h"
#include "RenderTarget.h"

#include "../Graphics/Graphics.h"

namespace Katame
{
	DepthStencil::DepthStencil(Graphics& gfx, UINT width, UINT height, bool canBindShaderInput)
	{
		// create depth stensil texture
		ID3D11Texture2D* pDepthStencil;
		D3D11_TEXTURE2D_DESC descDepth = {};
		descDepth.Width = width;
		descDepth.Height = height;
		descDepth.MipLevels = 1u;
		descDepth.ArraySize = 1u;
		descDepth.Format = DXGI_FORMAT_R32_TYPELESS; // this will need to be fixed
		descDepth.SampleDesc.Count = 1u;
		descDepth.SampleDesc.Quality = 0u;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | (canBindShaderInput ? D3D11_BIND_SHADER_RESOURCE : 0);
		gfx.m_Device->CreateTexture2D(&descDepth, nullptr, &pDepthStencil);
		
		CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D, DXGI_FORMAT_D32_FLOAT);

		// create target view of depth stensil texture
		HRESULT hr = gfx.m_Device->CreateDepthStencilView(
			pDepthStencil, &depthStencilViewDesc, &pDepthStencilView // nullptr will need to be replaced
		);
	}

	void DepthStencil::BindAsBuffer(Graphics& gfx) noexcept
	{
		gfx.m_Context->OMSetRenderTargets(0, nullptr, pDepthStencilView);
	}

	void DepthStencil::BindAsBuffer(Graphics& gfx, BufferResource* renderTarget) noexcept
	{
		assert(dynamic_cast<RenderTarget*>(renderTarget) != nullptr);
		BindAsBuffer(gfx, static_cast<RenderTarget*>(renderTarget));
	}

	void DepthStencil::BindAsBuffer(Graphics& gfx, RenderTarget* rt) noexcept
	{
		rt->BindAsBuffer( gfx, this );
	}

	void DepthStencil::Clear(Graphics& gfx) noexcept
	{
		gfx.m_Context->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);
	}


	ShaderInputDepthStencil::ShaderInputDepthStencil(Graphics& gfx, UINT slot)
		:
		ShaderInputDepthStencil(gfx, 0, 0, slot)
	{}

	ShaderInputDepthStencil::ShaderInputDepthStencil(Graphics& gfx, UINT width, UINT height, UINT slot)
		:
		DepthStencil(gfx, width, height, true),
		slot(slot)
	{
		ID3D11Resource* pRes;
		pDepthStencilView->GetResource(&pRes);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT; // this will need to be fixed
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		gfx.m_Device->CreateShaderResourceView(
			pRes, &srvDesc, &pShaderResourceView
		);
	}

	void ShaderInputDepthStencil::Bind(Graphics& gfx) noexcept
	{
		gfx.m_Context->PSSetShaderResources(slot, 1u, &pShaderResourceView);
	}


	OutputOnlyDepthStencil::OutputOnlyDepthStencil(Graphics& gfx)
		:
		OutputOnlyDepthStencil(gfx, 0, 0)
	{}

	OutputOnlyDepthStencil::OutputOnlyDepthStencil(Graphics& gfx, UINT width, UINT height)
		:
		DepthStencil(gfx, width, height, false)
	{}

	void OutputOnlyDepthStencil::Bind(Graphics& gfx) noexcept
	{
		assert("OutputOnlyDepthStencil cannot be bound as shader input" && false);
	}
}