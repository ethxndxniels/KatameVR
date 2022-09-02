#pragma once

#include "../XR/XRGraphics.h"

#include <string>

namespace Katame
{
	class VertexShader
	{
	public:
		VertexShader( const std::string& path );
		void Bind() noexcept;
		ID3DBlob* GetBytecode() const noexcept;
	protected:
		std::string m_sPath;
		ID3DBlob* m_pBytecodeBlob;
		ID3D11VertexShader* m_pVertexShader;
	};
}