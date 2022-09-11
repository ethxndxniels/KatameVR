#include "Topology.h"

#include "../Graphics/Graphics.h"

namespace Katame
{
	Topology::Topology( Graphics* gfx, D3D11_PRIMITIVE_TOPOLOGY type )
		: type( type )
	{}

	void Topology::Bind( Graphics* gfx )
	{
		gfx->m_Context->IASetPrimitiveTopology( type );
	}
}