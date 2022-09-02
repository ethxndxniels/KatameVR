#pragma once

#include "../XR/XRGraphics.h"

#include "VertexShader.h"

namespace Katame
{
	class InputLayout
	{
	public:
		InputLayout( std::vector<D3D11_INPUT_ELEMENT_DESC> positionLayout, const VertexShader& vs );
		void Bind();
	private:
		ID3D11InputLayout* m_InputLayout;
	};
}
