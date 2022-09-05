#include "VertexBuffer.h"
#include "BindableCodex.h"

#include "../Graphics/Mesh.h"

namespace Katame
{
	VertexBuffer::VertexBuffer( void* buffer, unsigned int size, unsigned int stride )
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
		XRGraphics::GetDevice()->CreateBuffer( &bd, &sd, &m_pVertexBuffer );
	}

	void VertexBuffer::Bind() noexcept
	{
		const UINT offset = 0u;
		XRGraphics::GetContext()->IASetVertexBuffers( 0u, 1u, &m_pVertexBuffer, &m_Stride, &offset );
	}

	IndexBuffer::IndexBuffer( void* buffer, unsigned int size, unsigned int stride )
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
		XRGraphics::GetDevice()->CreateBuffer( &ibd, &isd, &m_pIndexBuffer );
	}

	void IndexBuffer::Bind() noexcept
	{
		XRGraphics::GetContext()->IASetIndexBuffer( m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0u );
	}
}