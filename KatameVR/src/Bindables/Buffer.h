#pragma once

#include "../Graphics/Graphics.h"

#include <memory>

namespace Katame
{
	class VertexBuffer
	{
	public:
		VertexBuffer( Graphics* gfx, void* buffer, unsigned int size, unsigned int stride );
		void Bind( Graphics* gfx ) noexcept;
	private:
		UINT m_Stride;
		ID3D11Buffer* m_pVertexBuffer;
	};

	class IndexBuffer
	{
	public:
		IndexBuffer( Graphics* gfx, void* buffer, unsigned int size, unsigned int stride );
		void Bind( Graphics* gfx ) noexcept;
	private:
		UINT m_Stride;
		ID3D11Buffer* m_pIndexBuffer;
	};
}