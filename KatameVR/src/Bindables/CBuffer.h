#pragma once

#include "Bindable.h"

#include <d3d11.h>

namespace Katame
{
	class Graphics;

	class VCBuffer : public Bindable
	{
	public:
		VCBuffer( Graphics* gfx, const CD3D11_BUFFER_DESC& cbufDesc, unsigned int slot );
		void Update( Graphics* gfx, const void* data );
		void Bind( Graphics* gfx ) override;
	private:
		unsigned int u_Slot;
		ID3D11Buffer* m_CBuffer;
	};

	class PCBuffer : public Bindable
	{
	public:
		PCBuffer( Graphics* gfx, const CD3D11_BUFFER_DESC& cbufDesc, unsigned int slot );
		void Update( Graphics* gfx, const void* data );
		void Bind( Graphics* gfx ) override;
	private:
		unsigned int u_Slot;
		ID3D11Buffer* m_CBuffer;
	};
}