#pragma once

#include "Bindable.h"

#include <d3d11.h>

namespace Katame
{
	class Graphics;

	class Rasterizer : public Bindable
	{
	public:
		Rasterizer( Graphics& gfx, bool twoSided = false );
		void Bind( Graphics& gfx ) override;
	private:
		ID3D11RasterizerState* m_pRasterizer;
		bool m_TwoSided;
	};
}
