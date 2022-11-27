#include "PointLight.h"

#include "../Graphics/Graphics.h"

#include "../Drawable/Drawables/PointLightCore.h"

namespace Katame
{
	PointLight::PointLight(Graphics* gfx)
		: gfx( gfx )
	{
		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0u;
		desc.ByteWidth = sizeof(LightData);
		desc.StructureByteStride = 0u;

		m_Buffer = new PCBuffer(gfx, 3u, sizeof(LightData));
		m_LightCore = new PointLightCore(gfx);
	}
	PointLight::~PointLight()
	{
		delete m_Buffer;
	}
	void PointLight::Bind()
	{
		LightData data;
		XMStoreFloat3(&data.position, { m_BufferData.position.x,  m_BufferData.position.y,  m_BufferData.position.z });
		m_Buffer->Update(gfx, &data);
		m_Buffer->Bind(gfx);
		m_LightCore->SetData({ {}, { m_BufferData.position.x,  m_BufferData.position.y,  m_BufferData.position.z } }, {0.5f,0.5f,0.5f});
	}
	void PointLight::Update(float dt)
	{
		if (m_BufferData.position.x > 10.0f)
			flag = false;
		else if (m_BufferData.position.x < -10.0f)
			flag = true;

		if (flag)
			m_BufferData.position.x += 0.1f;
		else
			m_BufferData.position.x -= 0.1f;
	}
	Drawable* PointLight::GetLightCore()
	{
		return m_LightCore;
	}
}