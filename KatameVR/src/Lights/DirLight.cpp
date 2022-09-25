#include "DirLight.h"

#include "../Graphics/Graphics.h"

namespace Katame
{
	DirLight::DirLight( Graphics* gfx )
		: gfx( gfx )
	{
		CD3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0u;
		desc.ByteWidth = sizeof( LightData );
		desc.StructureByteStride = 0u;

		m_Buffer = new PCBuffer( gfx, desc, 2u );
	}

	DirLight::~DirLight()
	{
		delete m_Buffer;
	}

	void DirLight::Bind()
	{
		LightData data;
		XMStoreFloat4( &data.lightDir, { m_BufferData.lightDir.x,  m_BufferData.lightDir.y,  m_BufferData.lightDir.z,  m_BufferData.lightDir.w } );
		m_Buffer->Update( gfx, &data, sizeof( LightData ) );
		m_Buffer->Bind( gfx );
	}

	void DirLight::Update( float dt )
	{
		if (m_BufferData.lightDir.y > 1.0f)
			flag = false;
		else if (m_BufferData.lightDir.y < -1.0f)
			flag = true;

		if (flag)
			m_BufferData.lightDir.y += dt * 0.1f;
		else
			m_BufferData.lightDir.y -= dt * 0.1f;

	}

}