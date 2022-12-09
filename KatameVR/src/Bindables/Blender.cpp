#include "Blender.h"

#include "../Graphics/Graphics.h"

namespace Katame
{
	Blender::Blender( Graphics& gfx, bool blending )
		:
		blending( blending )
		{
			D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC{ CD3D11_DEFAULT{} };
			auto& brt = blendDesc.RenderTarget[0];
			if (blending)
			{
				brt.BlendEnable = TRUE;
				brt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
				brt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			}
			gfx.m_Device->CreateBlendState( &blendDesc, &m_Blender );
	}

	void Blender::Bind( Graphics& gfx )
	{
		gfx.m_Context->OMSetBlendState( m_Blender, nullptr, 0xFFFFFFFFu );
	}
}