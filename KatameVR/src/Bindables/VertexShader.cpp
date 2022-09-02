#include "VertexShader.h"

#include "../Utilities/KatameUtils.h"

namespace Katame
{
	VertexShader::VertexShader( const std::string& path )
		:
		m_sPath( path )
	{
		D3DReadFileToBlob( ToWide( path ).c_str(), &m_pBytecodeBlob );
		XRGraphics::GetDevice()->CreateVertexShader(
			m_pBytecodeBlob->GetBufferPointer(),
			m_pBytecodeBlob->GetBufferSize(),
			nullptr,
			&m_pVertexShader
		);
	}
	void VertexShader::Bind() noexcept
	{
		XRGraphics::GetContext()->VSSetShader( m_pVertexShader, nullptr, 0u );
	}
	ID3DBlob* VertexShader::GetBytecode() const noexcept
	{
		return m_pBytecodeBlob;
	}
}