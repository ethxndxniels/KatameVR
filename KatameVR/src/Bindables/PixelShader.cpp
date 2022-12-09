#include "PixelShader.h"

#include "../Utilities/KatameUtils.h"
#include "../Graphics/Graphics.h"

namespace Katame
{
	PixelShader::PixelShader( Graphics& gfx, const std::string& path )
		:
		m_sPath( path )
	{
		ID3DBlob* pBlob;
		D3DReadFileToBlob( ToWide( path ).c_str(), &pBlob );
		gfx.m_Device->CreatePixelShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pPixelShader );
	}
	void PixelShader::Bind( Graphics& gfx )
	{
		gfx.m_Context->PSSetShader( m_pPixelShader, nullptr, 0u );
	}
}
