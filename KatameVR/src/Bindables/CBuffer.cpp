#include "CBuffer.h"

namespace Katame
{
	VCBuffer::VCBuffer( const CD3D11_BUFFER_DESC& cbufDesc, unsigned int size, unsigned int slot )
		: u_Slot( slot )
	{
		XRGraphics::GetDevice()->CreateBuffer( &cbufDesc, nullptr, &m_CBuffer );
	}

	void VCBuffer::Update( const void* data )
	{
		XRGraphics::GetContext()->UpdateSubresource( m_CBuffer, u_Slot, nullptr, data, 0u, 0u );
	}

	void VCBuffer::Bind()
	{
		XRGraphics::GetContext()->VSSetConstantBuffers( u_Slot, 1u, &m_CBuffer );
	}

	PCBuffer::PCBuffer( const CD3D11_BUFFER_DESC& cbufDesc, unsigned int size, unsigned int slot )
		: u_Slot( slot )
	{
		XRGraphics::GetDevice()->CreateBuffer( &cbufDesc, nullptr, &m_CBuffer );
	}

	void PCBuffer::Update( const void* data )
	{
		XRGraphics::GetContext()->UpdateSubresource( m_CBuffer, u_Slot, nullptr, data, 0u, 0u );
	}

	void PCBuffer::Bind()
	{
		XRGraphics::GetContext()->PSSetConstantBuffers( u_Slot, 1u, &m_CBuffer );
	}
}