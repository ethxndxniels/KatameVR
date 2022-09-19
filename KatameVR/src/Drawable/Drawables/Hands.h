#pragma once

#include "../Drawable.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <openxr/openxr.h>

namespace Katame
{
	class Graphics;
	class XRCore;
	class Mesh;

	class Hands
	{
	public:
		Hands( Graphics* gfx, XRCore* xrCore );
		~Hands();
	public:
		void Update( float dt );
		Mesh* GetLeftHand();
		Mesh* GetRightHand();
	private:
		Graphics* gfx;
		XRCore* xrCore;
	private:
		Mesh* m_LeftHand;
		Mesh* m_RightHand;
		//ColorCube* m_LeftHand;
		//ColorCube* m_RightHand;
	};
};
