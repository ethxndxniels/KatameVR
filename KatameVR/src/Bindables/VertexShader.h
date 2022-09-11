#pragma once

#include "Bindable.h"

#include <string>

#include <d3d11.h>

namespace Katame
{
	class Graphics;

	class VertexShader : public Bindable
	{
	public:
		VertexShader( Graphics* gfx, const std::string& path );
		void Bind( Graphics* gfx ) override;
		ID3DBlob* GetBytecode() const noexcept;
	protected:
		std::string m_sPath;
		ID3DBlob* m_pBytecodeBlob;
		ID3D11VertexShader* m_pVertexShader;
	};
}