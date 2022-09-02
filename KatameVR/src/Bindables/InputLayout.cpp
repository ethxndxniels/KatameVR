#include "InputLayout.h"

namespace Katame
{
	InputLayout::InputLayout( std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayout, const VertexShader& vs )
	{
		XRGraphics::GetDevice()->CreateInputLayout(
			inputLayout.data(), (UINT)inputLayout.size(),
			vs.GetBytecode()->GetBufferPointer(),
			vs.GetBytecode()->GetBufferSize(),
			&m_InputLayout
		);
	}

	void InputLayout::Bind()
	{
		XRGraphics::GetContext()->IASetInputLayout( m_InputLayout );
	}
}