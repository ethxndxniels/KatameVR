#include "VertexBuffer.h"
#include "BindableCodex.h"

namespace Katame
{

	namespace Bind
	{

		VertexBuffer::VertexBuffer( Graphics* gfx, const Dvtx::VertexBuffer& vbuf )
			:
			VertexBuffer( gfx, "?", vbuf )
		{}
		VertexBuffer::VertexBuffer( Graphics* gfx, const std::string& tag, const Dvtx::VertexBuffer& vbuf )
			:
			m_iStride( (UINT)vbuf.GetLayout().Size() ),
			m_sTag( tag )
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
			gfx->m_Device->CreateBuffer( &bd, &sd, &m_pVertexBuffer );
		}
		void VertexBuffer::Bind( Graphics* gfx ) noexcept
		{
			const UINT offset = 0u;
			gfx->m_Context->IASetVertexBuffers( 0u, 1u, &m_pVertexBuffer, &m_iStride, &offset );
		}
		std::shared_ptr<VertexBuffer> VertexBuffer::Resolve( Graphics* gfx, const std::string& tag,
			const Dvtx::VertexBuffer& vbuf )
		{
			assert( tag != "?" );
			return Codex::Resolve<VertexBuffer>( gfx, tag, vbuf );
		}
		std::string VertexBuffer::GenerateUID_( const std::string& tag )
		{
			using namespace std::string_literals;
			return typeid(VertexBuffer).name() + "#"s + tag;
		}
		std::string VertexBuffer::GetUID() const noexcept
		{
			return GenerateUID( m_sTag );
		}

	}

}