#include "VertexShader.h"

namespace Katame
{
	std::wstring ToWide( const std::string& narrow )
	{
		wchar_t wide[512];
		mbstowcs_s( nullptr, wide, narrow.c_str(), _TRUNCATE );
		return wide;
	}

	VertexShader::VertexShader( ID3D11Device* device, const std::string& path )
		:
		m_sPath( path )
	{
		D3DReadFileToBlob( ToWide( path ).c_str(), &m_pBytecodeBlob );
		device->CreateVertexShader(
			m_pBytecodeBlob->GetBufferPointer(),
			m_pBytecodeBlob->GetBufferSize(),
			nullptr,
			&m_pVertexShader
		);
	}

	void VertexShader::Bind( ID3D11DeviceContext* context ) noexcept
	{
		context->VSSetShader( m_pVertexShader, nullptr, 0u );
	}

	ID3DBlob* VertexShader::GetBytecode() const noexcept
	{
		return m_pBytecodeBlob;
	}

}
