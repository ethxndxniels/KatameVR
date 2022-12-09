#include "DirLight.h"

#include "../Graphics/Graphics.h"

namespace Katame
{
	DirLight::DirLight( Graphics& gfx )
		: gfx( &gfx ), m_Buffer( gfx, 2u, sizeof(LightData) )
	{
	}

	DirLight::~DirLight()
	{
	}

	void DirLight::Bind()
	{
		LightData data;
		XMStoreFloat4( &data.lightDir, { m_BufferData.lightDir.x,  m_BufferData.lightDir.y,  m_BufferData.lightDir.z,  m_BufferData.lightDir.w } );
		m_Buffer.Update( *gfx, &data );
		m_Buffer.Bind( *gfx );
	}

	void DirLight::Update( float dt )
	{
		/*if (m_BufferData.lightDir.y > 0.0f)
			flag = false;
		else if (m_BufferData.lightDir.y < -1.0f)
			flag = true;

		if (flag)
			m_BufferData.lightDir.y += 0.01f;
		else
			m_BufferData.lightDir.y -= 0.01f;*/

	}

}