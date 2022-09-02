#pragma once

#include "../XR/XRGraphics.h"

namespace Katame
{
	class VCBuffer
	{
	public:
		VCBuffer( const CD3D11_BUFFER_DESC& cbufDesc, unsigned int size, unsigned int slot );
		void Update( const void* data );
		void Bind();
	private:
		unsigned int u_Slot;
		ID3D11Buffer* m_CBuffer;
	};

	class PCBuffer
	{
	public:
		PCBuffer( const CD3D11_BUFFER_DESC& cbufDesc, unsigned int size, unsigned int slot );
		void Update( const void* data );
		void Bind();
	private:
		unsigned int u_Slot;
		ID3D11Buffer* m_CBuffer;
	};
}
