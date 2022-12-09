#pragma once

#include "../Drawable.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <openxr/openxr.h>

namespace Katame
{
	class Graphics;

	class NormalCube : public Drawable
	{
	public:
		NormalCube( Graphics& gfx );
		~NormalCube();
	public:
		void Update( float dt );
		DirectX::XMMATRIX GetModelMatrix() override;
		void SetData( XrPosef pose, XrVector3f scale );
	private:
		XrPosef Pose;
		XrVector3f Scale;
	};
}
