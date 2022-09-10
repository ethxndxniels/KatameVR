#pragma once

#include "../Graphics/Graphics.h"

namespace Katame
{
	class VCBuffer
	{
	public:
		VCBuffer( Graphics* gfx, const CD3D11_BUFFER_DESC& cbufDesc, unsigned int slot );
		void Update( Graphics* gfx, const void* data );
		void Bind( Graphics* gfx );
	private:
		unsigned int u_Slot;
		ID3D11Buffer* m_CBuffer;
	};

	class PCBuffer
	{
	public:
		PCBuffer( Graphics* gfx, const CD3D11_BUFFER_DESC& cbufDesc, unsigned int slot );
		void Update( Graphics* gfx, const void* data );
		void Bind( Graphics* gfx );
	private:
		unsigned int u_Slot;
		ID3D11Buffer* m_CBuffer;
	};
}