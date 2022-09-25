#pragma once

#include "../Drawable.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <openxr/openxr.h>

namespace Katame
{
	class NormalCube;
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
		Drawable* GetLeftHand();
		Drawable* GetRightHand();
	private:
		Graphics* gfx;
		XRCore* xrCore;
	private:
		//Mesh* m_LeftHand;
		//Mesh* m_RightHand;
		NormalCube* m_LeftHand;
		NormalCube* m_RightHand;
	};
};
