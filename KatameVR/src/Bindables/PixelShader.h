#pragma once

#include "Bindable.h"

#include <string>

#include <d3d11.h>

namespace Katame
{
	class Graphics;

	class PixelShader : public Bindable
	{
	public:
		PixelShader( Graphics& gfx, const std::string& path );
		void Bind( Graphics& gfx ) override;
	protected:
		std::string m_sPath;
		ID3D11PixelShader* m_pPixelShader;
	};
}