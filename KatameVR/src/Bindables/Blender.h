#pragma once

#include "Bindable.h"

#include <d3d11.h>

namespace Katame
{
	class Graphics;

	class Blender : public Bindable
	{
	public:
		Blender( Graphics& gfx, bool blending );
		void Bind( Graphics& gfx ) override;
	private:
		ID3D11BlendState* m_Blender;
		bool blending;
	};
}