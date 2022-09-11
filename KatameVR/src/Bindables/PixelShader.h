#pragma once

#include <string>

#include <d3d11.h>

namespace Katame
{
	class Graphics;

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