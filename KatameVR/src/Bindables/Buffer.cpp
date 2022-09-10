#include "Buffer.h"

namespace Katame
{
	VertexBuffer::VertexBuffer( Graphics* gfx, void* buffer, unsigned int size, unsigned int stride )
		: m_Stride( stride )
	{
		D3D11_BUFFER_DESC bd = {};
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0u;
		bd.MiscFlags = 0u;
		bd.ByteWidth = size;
		bd.StructureByteStride = stride;
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = buffer;
		gfx->m_Device->CreateBuffer( &bd, &sd, &m_pVertexBuffer );
	}

	void VertexBuffer::Bind( Graphics* gfx ) noexcept
	{
		const UINT offset = 0u;
		gfx->m_Context->IASetVertexBuffers( 0u, 1u, &m_pVertexBuffer, &m_Stride, &offset );
	}

	IndexBuffer::IndexBuffer( Graphics* gfx, void* buffer, unsigned int size, unsigned int stride )
	{
		D3D11_BUFFER_DESC ibd = {};
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.Usage = D3D11_USAGE_DEFAULT;
		ibd.CPUAccessFlags = 0u;
		ibd.MiscFlags = 0u;
		ibd.ByteWidth = size;
		ibd.StructureByteStride = stride;
		D3D11_SUBRESOURCE_DATA isd = {};
		isd.pSysMem = buffer;
		gfx->m_Device->CreateBuffer( &ibd, &isd, &m_pIndexBuffer );
	}

	void IndexBuffer::Bind( Graphics* gfx ) noexcept
	{
		gfx->m_Context->IASetIndexBuffer( m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0u );
	}
}