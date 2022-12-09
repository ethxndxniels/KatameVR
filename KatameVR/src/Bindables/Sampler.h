#pragma once

#include "Bindable.h"

#include <d3d11.h>

#include <string>

namespace Katame
{
	class Graphics;
	
	class Sampler : public Bindable
	{
	public:
		enum class Type
		{
			Anisotropic,
			Bilinear,
			Point,
		};
	public:
		Sampler( Graphics& gfx, Type type, bool reflect );
		void Bind( Graphics& gfx ) override;
	protected:
		ID3D11SamplerState* pSampler;
		Type type;
		bool reflect;
	};
}