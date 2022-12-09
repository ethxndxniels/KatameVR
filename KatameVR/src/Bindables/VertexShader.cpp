#include "VertexShader.h"

#include "../Utilities/KatameUtils.h"
#include "../Graphics/Graphics.h"

#include <d3dcompiler.h>

namespace Katame
{
	VertexShader::VertexShader( Graphics& gfx, const std::string& path )
		:
		m_sPath( path )
	{
		D3DReadFileToBlob( ToWide( path ).c_str(), &m_pBytecodeBlob );
		gfx.m_Device->CreateVertexShader(
			m_pBytecodeBlob->GetBufferPointer(),
			m_pBytecodeBlob->GetBufferSize(),
			nullptr,
			&m_pVertexShader
		);
	}
	void VertexShader::Bind( Graphics& gfx )
	{
		gfx.m_Context->VSSetShader( m_pVertexShader, nullptr, 0u );
	}
	ID3DBlob* VertexShader::GetBytecode() const noexcept
	{
		return m_pBytecodeBlob;
	}
}
