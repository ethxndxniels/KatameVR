#include "CBuffer.h"

#include "../Graphics/Graphics.h"

namespace Katame
{
	VCBuffer::VCBuffer( Graphics* gfx, const CD3D11_BUFFER_DESC& cbufDesc, unsigned int slot )
		: u_Slot( slot )
	{
		gfx->m_Device->CreateBuffer( &cbufDesc, nullptr, &m_CBuffer );
	}
	void VCBuffer::Update( Graphics* gfx, const void* data )
	{
		gfx->m_Context->UpdateSubresource( m_CBuffer, 0, nullptr, data, 0u, 0u );
	}

	void VCBuffer::Update( Graphics* gfx, const void* data, size_t size )
	{
		D3D11_MAPPED_SUBRESOURCE msr;
		gfx->m_Context->Map(
			m_CBuffer, 0u,
			D3D11_MAP_WRITE_DISCARD, 0u,
			&msr
		);
		memcpy( msr.pData, &data, size );
		gfx->m_Context->Unmap( m_CBuffer, 0u );
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

	void PCBuffer::Update( Graphics* gfx, const void* data )
	{
		gfx->m_Context->UpdateSubresource( m_CBuffer, 0, nullptr, data, 0u, 0u );
	}

	void PCBuffer::Update( Graphics* gfx,  const void* data, size_t size )
	{
		D3D11_MAPPED_SUBRESOURCE msr;
		HRESULT hr = gfx->m_Context->Map(
			m_CBuffer, 0u,
			D3D11_MAP_WRITE_DISCARD, 0u,
			&msr
		);
		memcpy( msr.pData, &data, size );
		gfx->m_Context->Unmap( m_CBuffer, 0u );
	}

	void PCBuffer::Bind( Graphics* gfx )
	{
		gfx->m_Context->PSSetConstantBuffers( u_Slot, 1u, &m_CBuffer );
	}
}