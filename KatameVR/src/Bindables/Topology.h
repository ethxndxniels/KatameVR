#pragma once

#include "../XR/XRGraphics.h"

namespace Katame
{
	class Topology
	{
	public:
		Topology( D3D11_PRIMITIVE_TOPOLOGY type );
		void Bind();
	private:
		D3D11_PRIMITIVE_TOPOLOGY m_Type;
	};
}
