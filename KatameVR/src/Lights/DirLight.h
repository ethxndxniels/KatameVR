#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

#include "../Bindables/CBuffer.h"

namespace Katame
{
	class Graphics;

	class DirLight
	{
	public:
		DirLight( Graphics& gfx );
		~DirLight();
		void Bind();
		void Update( float dt );
	private:
		Graphics* gfx;
	private:
		struct LightData
		{
			DirectX::XMFLOAT4 lightDir = { 0.0f, -1.0f, 0.0f, 0.0f };
		};
		LightData m_BufferData;
		PCBuffer m_Buffer;
		bool flag = true;
	};
}