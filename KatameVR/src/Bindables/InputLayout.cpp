#include "InputLayout.h"
#include "BindableCodex.h"
#include "../Graphics/Vertex.h"

namespace Katame
{
	
	namespace Bind
	{

		InputLayout::InputLayout( Graphics* gfx,
			Dvtx::VertexLayout layout_in,
			ID3DBlob* pVertexShaderBytecode )
			:
			layout( std::move( layout_in ) )
		{
			const auto d3dLayout = layout.GetD3DLayout();

			gfx->m_Device->CreateInputLayout(
				d3dLayout.data(), (UINT)d3dLayout.size(),
				pVertexShaderBytecode->GetBufferPointer(),
				pVertexShaderBytecode->GetBufferSize(),
				&pInputLayout
			);
		}

		void InputLayout::Bind( Graphics* gfx ) noexcept
		{
			gfx->m_Context->IASetInputLayout( pInputLayout );
		}

		std::shared_ptr<InputLayout> InputLayout::Resolve( Graphics* gfx, 
			const Dvtx::VertexLayout& layout, ID3DBlob* pVertexShaderBytecode )
		{
			return Codex::Resolve<InputLayout>( gfx, layout, pVertexShaderBytecode );
		}
		std::string InputLayout::GenerateUID( const Dvtx::VertexLayout& layout, ID3DBlob* pVertexShaderBytecode )
		{
			using namespace std::string_literals;
			return typeid(InputLayout).name() + "#"s + layout.GetCode();
		}
		std::string InputLayout::GetUID() const noexcept
		{
			return GenerateUID( layout );
		}
	}
}