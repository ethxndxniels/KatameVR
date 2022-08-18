#include "VertexBuffer.h"

namespace Katame
{

	VertexBuffer::VertexBuffer( ID3D11Device* device, const Dvtx::VertexBuffer& vbuf )
		:
		VertexBuffer( device, "?", vbuf )
	{}

	VertexBuffer::VertexBuffer( ID3D11Device* device, const std::string& tag, const Dvtx::VertexBuffer& vbuf )
		:
		stride( (UINT)vbuf.GetLayout().Size() )
	{
		D3D11_BUFFER_DESC bd = {};
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0u;
		bd.MiscFlags = 0u;
		bd.ByteWidth = UINT( vbuf.SizeBytes() );
		bd.StructureByteStride = stride;
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = vbuf.GetData();
		device->CreateBuffer( &bd, &sd, &pVertexBuffer );
	}

	void VertexBuffer::Bind( ID3D11DeviceContext* context ) noexcept
	{
		const UINT offset = 0u;
		context->IASetVertexBuffers( 0u, 1u, &pVertexBuffer, &stride, &offset );
		context->IASetVertexBuffers( 0u, 1u, &pVertexBuffer, &stride, &offset );
	}

}