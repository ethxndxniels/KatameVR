#pragma once

#include <memory>

#include <d3d11.h>

namespace Katame
{
	class Graphics;

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