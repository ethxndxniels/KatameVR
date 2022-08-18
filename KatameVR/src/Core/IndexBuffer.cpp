#include "IndexBuffer.h"

namespace Katame
{

	IndexBuffer::IndexBuffer( ID3D11Device* device, const std::vector<unsigned short>& indices )
		:
		IndexBuffer( device, "?", indices )
	{}
	IndexBuffer::IndexBuffer( ID3D11Device* device, std::string tag, const std::vector<unsigned short>& indices )
		:
		tag( tag ),
		count( (UINT)indices.size() )
	{
		D3D11_BUFFER_DESC ibd = {};
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.Usage = D3D11_USAGE_DEFAULT;
		ibd.CPUAccessFlags = 0u;
		ibd.MiscFlags = 0u;
		ibd.ByteWidth = UINT( count * sizeof( unsigned short ) );
		ibd.StructureByteStride = sizeof( unsigned short );
		D3D11_SUBRESOURCE_DATA isd = {};
		isd.pSysMem = indices.data();
		device->CreateBuffer( &ibd, &isd, &pIndexBuffer );
	}
	void IndexBuffer::Bind( ID3D11DeviceContext* context ) noexcept
	{
		context->IASetIndexBuffer( pIndexBuffer, DXGI_FORMAT_R16_UINT, 0u );
	}

}