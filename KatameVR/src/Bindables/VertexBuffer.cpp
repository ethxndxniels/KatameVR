#include "VertexBuffer.h"
#include "BindableCodex.h"

namespace Katame
{

	namespace Bind
	{
		VertexBuffer::VertexBuffer( unsigned int size )
			:  m_Size( size )
		{
			D3D11_BUFFER_DESC bd = {};
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.CPUAccessFlags = 0u;
			bd.MiscFlags = 0u;
			bd.ByteWidth = UINT( vbuf.SizeBytes() );
			bd.StructureByteStride = m_iStride;
			D3D11_SUBRESOURCE_DATA sd = {};
			sd.pSysMem = vbuf.GetData();
			ID3D11Buffer test = gfx->m_Device->CreateBuffer( &bd, &sd, &m_pVertexBuffer );

			//KM_RENDER_S( {
			//	glGenBuffers( 1, &self->m_RendererID );
			//	} );
		}

		VertexBuffer::~VertexBuffer()
		{
			//KM_RENDER_S( {
			//	glDeleteBuffers( 1, &self->m_RendererID );
			//	} );
		}

		void VertexBuffer::SetData( void* buffer, unsigned int size, unsigned int offset )
		{
			m_Size = size;
			//KM_RENDER_S3( buffer, size, offset, {
			//	glBindBuffer( GL_ARRAY_BUFFER, self->m_RendererID );
			//	glBufferData( GL_ARRAY_BUFFER, size, buffer, GL_STATIC_DRAW );
			//	} );
		}

		void VertexBuffer::Bind() const
		{
			//KM_RENDER_S( {
			//	glBindBuffer( GL_ARRAY_BUFFER, self->m_RendererID );

			// TODO: Extremely temp, by default provide positions and texcoord attributes
			//glEnableVertexAttribArray( 0 );
			//glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( float ) * 5, 0 );

			//glEnableVertexAttribArray( 1 );
			//glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 5, (const void*)(3 * sizeof( float )) );
			//	} );
		}


	}

}