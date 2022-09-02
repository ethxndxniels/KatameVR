#pragma once

#include "../XR/XRGraphics.h"

#include <string>

namespace Katame
{
	class PixelShader
	{
	public:
		PixelShader( const std::string& path );
		void Bind() noexcept;
	protected:
		std::string m_sPath;
		ID3D11PixelShader* m_pPixelShader;
	};
}