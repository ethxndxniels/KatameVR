#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>

#include <string>

namespace Katame
{
	class VertexShader
	{
	public:
		VertexShader( ID3D11Device* device, const std::string& path );
		void Bind( ID3D11DeviceContext* context ) noexcept;
		ID3DBlob* GetBytecode() const noexcept;
	protected:
		std::string m_sPath;
		ID3DBlob* m_pBytecodeBlob;
		ID3D11VertexShader* m_pVertexShader;
	};
}