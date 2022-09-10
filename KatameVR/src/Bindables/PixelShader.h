#pragma once

#include "../Graphics/Graphics.h"

#include <string>

namespace Katame
{
	class PixelShader
	{
	public:
		PixelShader( Graphics* gfx, const std::string& path );
		void Bind( Graphics* gfx ) noexcept;
	protected:
		std::string m_sPath;
		ID3D11PixelShader* m_pPixelShader;
	};
}