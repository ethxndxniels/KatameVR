#pragma once

#include <d3d11.h>

#include "Vertex.h"

namespace Katame
{

	class InputLayout
	{
	public:
		InputLayout( ID3D11Device* device,
			Dvtx::VertexLayout layout,
			ID3DBlob* pVertexShaderBytecode );
		void Bind( ID3D11DeviceContext* context ) noexcept ;
	protected:
		Dvtx::VertexLayout layout;
		ID3D11InputLayout* pInputLayout;
	};

}

