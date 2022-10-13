#include "Texture.h"

#include "../Graphics/Graphics.h"
#include "../Renderer/Surface.h"
#include "../Utilities/KatameUtils.h"

namespace Katame
{
	Texture::Texture( Graphics* gfx, const std::string& path, UINT slot )
	{
		// load surface
		const auto s = Surface::FromFile( path );
		hasAlpha = s.AlphaLoaded();

		// create texture resource
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = s.GetWidth();
		textureDesc.Height = s.GetHeight();
		textureDesc.MipLevels = 0;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		ID3D11Texture2D* pTexture;
		gfx->m_Device->CreateTexture2D(
			&textureDesc, nullptr, &pTexture
		);

		// write image data into top mip level
		gfx->m_Context->UpdateSubresource(
			pTexture, 0u, nullptr, s.GetBufferPtrConst(), s.GetWidth() * sizeof( Surface::Color ), 0u
		);

		// create the resource view on the texture
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = -1;
		gfx->m_Device->CreateShaderResourceView(
			pTexture, &srvDesc, &pTextureView
		);

		// generate the mip chain using the gpu rendering pipeline
		gfx->m_Context->GenerateMips( pTextureView );
	}

	void Texture::Bind( Graphics* gfx )
	{
		gfx->m_Context->PSSetShaderResources( slot, 1u, &pTextureView );
	}

}