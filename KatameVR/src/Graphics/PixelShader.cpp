#include "PixelShader.h"

namespace Katame
{
	std::wstring ToWide2( const std::string& narrow )
	{
		wchar_t wide[512];
		mbstowcs_s( nullptr, wide, narrow.c_str(), _TRUNCATE );
		return wide;
	}

	PixelShader::PixelShader( ID3D11Device* device, const std::string& path )
		:
		m_sPath( path )
	{
		D3DReadFileToBlob( ToWide2( path ).c_str(), &m_pBytecodeBlob );
		device->CreatePixelShader(
			m_pBytecodeBlob->GetBufferPointer(),
			m_pBytecodeBlob->GetBufferSize(),
			nullptr,
			&m_pPixelShader
		);
	}

	void PixelShader::Bind( ID3D11DeviceContext* context ) noexcept
	{
		context->PSSetShader( m_pPixelShader, nullptr, 0u );
	}

	ID3DBlob* PixelShader::GetBytecode() const noexcept
	{
		return m_pBytecodeBlob;
	}

}
