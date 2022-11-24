#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

#include "../Bindables/CBuffer.h"

namespace Katame
{
	class Graphics;

	class PointLight
	{
	public:
		PointLight(Graphics* gfx);
		~PointLight();
		void Bind();
		void Update(float dt);
	private:
		Graphics* gfx;
	private:
		struct LightData
		{
			DirectX::XMFLOAT3 position = { 0.0f, 5.0f, 0.0f };
			DirectX::XMFLOAT3 color = { 1.0f, 1.0f, 1.0f };
			DirectX::XMFLOAT2 padding = { 1.0f, 1.0f };
		};
		LightData m_BufferData;
		PCBuffer* m_Buffer = nullptr;
		bool flag = true;
	};
}