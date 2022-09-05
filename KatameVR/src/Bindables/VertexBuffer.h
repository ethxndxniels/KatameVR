#pragma once

#include "Bindable.h"
#include "../Graphics/Vertex.h"
#include <memory>

namespace Katame
{
	class VertexBuffer
	{
	public:
		VertexBuffer( Graphics* gfx, void* buffer, unsigned int size, unsigned int stride );
		void Bind() noexcept;
	private:
		UINT m_Stride;
		ID3D11Buffer* m_pVertexBuffer;
	};

	class IndexBuffer
	{
	public:
		IndexBuffer( void* buffer, unsigned int size, unsigned int stride );
		void Bind() noexcept;
	private:
		UINT m_Stride;
		ID3D11Buffer* m_pIndexBuffer;
	};
}