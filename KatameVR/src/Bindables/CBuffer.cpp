#include "CBuffer.h"

namespace Katame
{
	VCBuffer::VCBuffer( Graphics* gfx, const CD3D11_BUFFER_DESC& cbufDesc, unsigned int slot )
		: u_Slot( slot )
	{
		gfx->m_Device->CreateBuffer( &cbufDesc, nullptr, &m_CBuffer );
	}

	void VCBuffer::Update( Graphics* gfx, const void* data )
	{
		gfx->m_Context->UpdateSubresource( m_CBuffer, u_Slot, nullptr, data, 0u, 0u );
	}

	void VCBuffer::Bind( Graphics* gfx )
	{
		gfx->m_Context->VSSetConstantBuffers( u_Slot, 1u, &m_CBuffer );
	}

	PCBuffer::PCBuffer( Graphics* gfx, const CD3D11_BUFFER_DESC& cbufDesc, unsigned int slot )
		: u_Slot( slot )
	{
		gfx->m_Device->CreateBuffer( &cbufDesc, nullptr, &m_CBuffer );
	}

	void PCBuffer::Update( Graphics* gfx,  const void* data )
	{
		gfx->m_Context->UpdateSubresource( m_CBuffer, u_Slot, nullptr, data, 0u, 0u );
	}

	void PCBuffer::Bind( Graphics* gfx )
	{
		gfx->m_Context->PSSetConstantBuffers( u_Slot, 1u, &m_CBuffer );
	}
}