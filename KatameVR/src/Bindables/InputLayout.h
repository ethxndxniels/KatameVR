#pragma once

#include "../Graphics/Vertex.h"
#include "VertexShader.h"

namespace Katame
{
	class InputLayout
	{
	public:
		InputLayout( Graphics* gfx, std::vector<D3D11_INPUT_ELEMENT_DESC> positionLayout, const VertexShader& vs );
		void Bind( Graphics* gfx );
	private:
		ID3D11InputLayout* m_InputLayout;
	};
}

