#pragma once

#include "Bindable.h"

#include <d3d11.h>

namespace Katame
{
	class Graphics;

	class VCBuffer : public Bindable
	{
	public:
		VCBuffer( Graphics& gfx, unsigned int slot, size_t size );
		void Update( Graphics& gfx, const void* data );
		void Bind( Graphics& gfx ) override;
	private:
		unsigned int u_Slot;
		size_t u_Size;
		ID3D11Buffer* m_CBuffer;
	};

	class PCBuffer : public Bindable
	{
	public:
		PCBuffer( Graphics& gfx, unsigned int slot, size_t size );
		void Update( Graphics& gfx, const void* data );
		void Bind( Graphics& gfx ) override;
	private:
		unsigned int u_Slot;
		size_t u_Size;
		ID3D11Buffer* m_CBuffer;
	};
}
