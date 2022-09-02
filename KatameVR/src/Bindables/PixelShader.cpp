#include "PixelShader.h"

#include "../Utilities/KatameUtils.h"

namespace Katame
{
	PixelShader::PixelShader( const std::string& path )
		:
		m_sPath( path )
	{
		ID3DBlob* pBlob;
		D3DReadFileToBlob( ToWide( path ).c_str(), &pBlob );
		XRGraphics::GetDevice()->CreatePixelShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pPixelShader );
	}
	void PixelShader::Bind() noexcept
	{
		XRGraphics::GetContext()->PSSetShader( m_pPixelShader, nullptr, 0u );
	}
}