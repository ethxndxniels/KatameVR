#include "CBuffer.h"

#include "../Graphics/Graphics.h"

namespace Katame
{
	VCBuffer::VCBuffer( Graphics* gfx, unsigned int slot, size_t size )
		: u_Slot( slot ), u_Size( size )
	{
		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0u;
		desc.ByteWidth = (UINT)size;
		desc.StructureByteStride = 0u;

		gfx->m_Device->CreateBuffer( &desc, nullptr, &m_CBuffer );
	}


	void VCBuffer::Update( Graphics* gfx, const void* data )
	{
		D3D11_MAPPED_SUBRESOURCE msr;
		gfx->m_Context->Map(
			m_CBuffer, 0u,
			D3D11_MAP_WRITE_DISCARD, 0u,
			&msr
		);
		memcpy( msr.pData, data, u_Size );
		gfx->m_Context->Unmap( m_CBuffer, 0u );
	}

	void VCBuffer::Bind( Graphics* gfx )
	{
		gfx->m_Context->VSSetConstantBuffers( u_Slot, 1u, &m_CBuffer );
	}

	PCBuffer::PCBuffer( Graphics* gfx, unsigned int slot, size_t size )
		: u_Slot( slot ), u_Size( size )
	{
		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0u;
		desc.ByteWidth = (UINT)size;
		desc.StructureByteStride = 0u;

		gfx->m_Device->CreateBuffer( &desc, nullptr, &m_CBuffer );
	}

	void PCBuffer::Update( Graphics* gfx,  const void* data )
	{
		D3D11_MAPPED_SUBRESOURCE msr;
		gfx->m_Context->Map(
			m_CBuffer, 0u,
			D3D11_MAP_WRITE_DISCARD, 0u,
			&msr
		);
		memcpy( msr.pData, data, u_Size );
		gfx->m_Context->Unmap( m_CBuffer, 0u );
	}

	void PCBuffer::Bind( Graphics* gfx )
	{
		gfx->m_Context->PSSetConstantBuffers( u_Slot, 1u, &m_CBuffer );
	}
}