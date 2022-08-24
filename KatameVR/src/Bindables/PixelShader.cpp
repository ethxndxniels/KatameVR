#include "PixelShader.h"
#include "BindableCodex.h"
#include "../Utilities/KatameUtils.h"
namespace Katame
{
	namespace Bind
	{
		PixelShader::PixelShader( Graphics* gfx, const std::string& path )
			:
			m_sPath( path )
		{

			ID3DBlob* pBlob;
			D3DReadFileToBlob( ToWide( path ).c_str(), &pBlob );
			gfx->m_Device->CreatePixelShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pPixelShader );
		}
		void PixelShader::Bind( Graphics* gfx ) noexcept
		{
			gfx->m_Context->PSSetShader( m_pPixelShader, nullptr, 0u );
		}
		std::shared_ptr<PixelShader> PixelShader::Resolve( Graphics* gfx, const std::string& path )
		{
			return Codex::Resolve<PixelShader>( gfx, path );
		}
		std::string PixelShader::GenerateUID( const std::string& path )
		{
			using namespace std::string_literals;
			return typeid(PixelShader).name() + "#"s + path;
		}
		std::string PixelShader::GetUID() const noexcept
		{
			return GenerateUID( m_sPath );
		}
	}
}
