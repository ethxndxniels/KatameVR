#pragma once

#include "Vertex.h"
#include <memory>

namespace Katame
{
	class VertexBuffer
	{
	public:
		VertexBuffer( ID3D11Device* device, const std::string& tag, const Dvtx::VertexBuffer& vbuf );
		VertexBuffer( ID3D11Device* device, const Dvtx::VertexBuffer& vbuf );
		void Bind( ID3D11DeviceContext* context ) noexcept;
	private:
		std::string tag;
		UINT stride;
		ID3D11Buffer* pVertexBuffer;
	};
}