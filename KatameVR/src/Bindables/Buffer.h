#pragma once

#include "Bindable.h"

#include <memory>

#include <d3d11.h>

namespace Katame
{
	class Graphics;

	class VertexBuffer : public Bindable
	{
	public:
		VertexBuffer( Graphics& gfx, void* buffer, unsigned int size, unsigned int stride );
		void Bind( Graphics& gfx ) override;
	private:
		UINT m_Stride;
		ID3D11Buffer* m_pVertexBuffer;
	};

	class IndexBuffer : public Bindable
	{
	public:
		IndexBuffer( Graphics& gfx, void* buffer, unsigned int size, unsigned int stride );
		UINT GetCount() const noexcept;
		void Bind( Graphics& gfx ) override;
	private:
		UINT m_Count;
		ID3D11Buffer* m_pIndexBuffer;
	};
}