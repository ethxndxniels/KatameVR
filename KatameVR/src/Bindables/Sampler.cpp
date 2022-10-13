#include "Sampler.h"

#include "../Graphics/Graphics.h"

namespace Katame
{
	Sampler::Sampler( Graphics* gfx, Type type, bool reflect )
		:
		type( type ),
		reflect( reflect )
	{
		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };
		samplerDesc.Filter = [type]() {
			switch (type)
			{
			case Type::Anisotropic: return D3D11_FILTER_ANISOTROPIC;
			case Type::Point: return D3D11_FILTER_MIN_MAG_MIP_POINT;
			default:
			case Type::Bilinear: return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			}
		}();
		samplerDesc.AddressU = reflect ? D3D11_TEXTURE_ADDRESS_MIRROR : D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = reflect ? D3D11_TEXTURE_ADDRESS_MIRROR : D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
		gfx->m_Device->CreateSamplerState( &samplerDesc, &pSampler );
	}
	void Sampler::Bind( Graphics* gfx )
	{
		gfx->m_Context->PSSetSamplers( 0, 1, &pSampler );
	}
}