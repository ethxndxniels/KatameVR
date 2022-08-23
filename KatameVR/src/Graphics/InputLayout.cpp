#include "InputLayout.h"

namespace Katame
{

	InputLayout::InputLayout( ID3D11Device* device,
		Dvtx::VertexLayout layout_in,
		ID3DBlob* pVertexShaderBytecode )
		:
		layout( std::move( layout_in ) )
	{
		const auto d3dLayout = layout.GetD3DLayout();

		device->CreateInputLayout(
			d3dLayout.data(), (UINT)d3dLayout.size(),
			pVertexShaderBytecode->GetBufferPointer(),
			pVertexShaderBytecode->GetBufferSize(),
			&pInputLayout
		);
	}

	void InputLayout::Bind( ID3D11DeviceContext* context ) noexcept
	{
		context->IASetInputLayout( pInputLayout );
	}

}