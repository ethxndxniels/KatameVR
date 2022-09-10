#include "InputLayout.h"

namespace Katame
{
	InputLayout::InputLayout( Graphics* gfx, std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayout, const VertexShader& vs )
	{
		gfx->m_Device->CreateInputLayout(
			inputLayout.data(), (UINT)inputLayout.size(),
			vs.GetBytecode()->GetBufferPointer(),
			vs.GetBytecode()->GetBufferSize(),
			&m_InputLayout
		);
	}

	void InputLayout::Bind( Graphics* gfx )
	{
		gfx->m_Context->IASetInputLayout( m_InputLayout );
	}
}