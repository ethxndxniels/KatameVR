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
	class Model;

	class Hands
	{
	public:
		Hands( Graphics* gfx, XRCore* xrCore );
		~Hands();
	public:
		void Update( float dt );
		Model* GetLeftHand();
		Model* GetRightHand();
	private:
		Graphics* gfx;
		XRCore* xrCore;
	private:
		Model* m_LeftHand;
		Model* m_RightHand;
		//NormalCube* m_LeftHand;
		//NormalCube* m_RightHand;
	};
};
