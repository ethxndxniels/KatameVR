#include "PointLight.h"

#include "../Graphics/Graphics.h"

namespace Katame
{
	PointLight::PointLight(Graphics& gfx)
		: 
		gfx( &gfx ),
		m_Buffer( gfx, 3u, sizeof(LightData) ),
		m_LightCore( gfx ),
		m_ViewProjCBuf( gfx, 1u, sizeof(DirectX::XMFLOAT4X4) )
	{
	}
	PointLight::~PointLight()
	{
	}
	void PointLight::Bind()
	{
		LightData data;
		XMStoreFloat3(&data.position, { m_BufferData.position.x,  m_BufferData.position.y,  m_BufferData.position.z });
		m_Buffer.Update( *gfx, &data);
		m_Buffer.Bind( *gfx );
		m_LightCore.SetData({ {}, { m_BufferData.position.x,  m_BufferData.position.y,  m_BufferData.position.z } }, {0.5f,0.5f,0.5f});
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
	void PointLight::BindViewProj()
	{
		const DirectX::XMVECTOR forwardBaseVector = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		// apply the camera rotations to a base vector
		const auto lookVector = XMVector3Transform(forwardBaseVector,
			DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f)
		);
		// generate camera transform (applied to all objects to arrange them relative
		// to camera position/orientation in world) from cam position and direction
		// camera "top" always faces towards +Y (cannot do a barrel roll)
		const auto camPosition = DirectX::XMLoadFloat3(&m_BufferData.position);
		const auto camTarget = DirectX::XMVectorAdd( camPosition, lookVector );
		DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(camPosition, camTarget, DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
		DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveLH( 1.0f, 9.0f / 16.0f, 0.5f, 400.0f );
		DirectX::XMMATRIX viewProj = view * proj;

		m_ViewProjCBuf.Update( *gfx, &viewProj);
		m_ViewProjCBuf.Bind( *gfx );
	}
	Drawable& PointLight::GetLightCore()
	{
		return m_LightCore;
	}
}