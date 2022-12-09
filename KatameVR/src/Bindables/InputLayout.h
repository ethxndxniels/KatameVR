#pragma once

#include "Bindable.h"
#include "VertexShader.h"

#include <vector>

#include <d3d11.h>

namespace Katame
{
	class Graphics;

	class InputLayout : public Bindable
	{
	public:
		InputLayout( Graphics& gfx, std::vector<D3D11_INPUT_ELEMENT_DESC> positionLayout, const VertexShader& vs );
		void Bind( Graphics& gfx ) override;
	private:
		ID3D11InputLayout* m_InputLayout;
	};
}

