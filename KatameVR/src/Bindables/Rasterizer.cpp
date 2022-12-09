#include "Rasterizer.h"

#include "../Graphics/Graphics.h"

namespace Katame
{
	Rasterizer::Rasterizer( Graphics& gfx, bool twoSided )
		:
		m_TwoSided( twoSided )
	{
		D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC( CD3D11_DEFAULT{} );
		rasterDesc.CullMode = twoSided ? D3D11_CULL_NONE : D3D11_CULL_BACK;

		gfx.m_Device->CreateRasterizerState( &rasterDesc, &m_pRasterizer );
	}

	void Rasterizer::Bind( Graphics& gfx )
	{
		gfx.m_Context->RSSetState( m_pRasterizer );
	}
}