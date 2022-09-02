#include "Topology.h"

namespace Katame
{
	Topology::Topology( D3D11_PRIMITIVE_TOPOLOGY type )
		: m_Type( type )
	{
	}

	void Topology::Bind()
	{
		XRGraphics::GetContext()->IASetPrimitiveTopology( m_Type );
	}

}