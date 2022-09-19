#pragma once

#include "../Drawables/ColorCube.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <openxr/openxr.h>

namespace Katame
{
	class Graphics;
	class XRCore;

	class Hands
	{
	public:
		Hands( Graphics* gfx, XRCore* xrCore );
		~Hands();
	public:
		void Update( float dt );
		Drawable* GetLeftHand();
		Drawable* GetRightHand();
	private:
		Graphics* gfx;
		XRCore* xrCore;
	private:
		ColorCube* m_LeftHand;
		ColorCube* m_RightHand;
	};
};
