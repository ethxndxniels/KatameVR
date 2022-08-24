#include "Rasterizer.h"
#include "BindableCodex.h"

namespace Katame
{
	namespace Bind
	{
		Rasterizer::Rasterizer( Graphics* gfx, bool twoSided )
			:
			m_bTwoSided( twoSided )
		{
			D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC( CD3D11_DEFAULT{} );
			rasterDesc.CullMode = twoSided ? D3D11_CULL_NONE : D3D11_CULL_BACK;

			gfx->m_Device->CreateRasterizerState( &rasterDesc, &m_pRasterizer );
		}

		void Rasterizer::Bind( Graphics* gfx ) noexcept
		{
			gfx->m_Context->RSSetState( m_pRasterizer );
		}

		std::shared_ptr<Rasterizer> Rasterizer::Resolve( Graphics* gfx, bool twoSided )
		{
			return Codex::Resolve<Rasterizer>( gfx, twoSided );
		}
		std::string Rasterizer::GenerateUID( bool twoSided )
		{
			using namespace std::string_literals;
			return typeid(Rasterizer).name() + "#"s + (twoSided ? "2s" : "1s");
		}
		std::string Rasterizer::GetUID() const noexcept
		{
			return GenerateUID( m_bTwoSided );
		}
	}
}