#include "Drawable.h"

#include "../Graphics/Graphics.h"

#include "../Bindables/Bindable.h"
#include "../Bindables/Buffer.h"
#include "../Bindables/InputLayout.h"
#include "../Bindables/Topology.h"

#include <memory>

namespace Katame
{
	Drawable::~Drawable()
	{
		for (Bindable* bindable : binds)
			delete bindable;
	}
	void Drawable::AddBind( Bindable* bind )
	{
		binds.push_back( bind );
	}
	void Drawable::Render( Graphics* gfx )
	{
		for (Bindable* bind : binds)
		{
			bind->Bind( gfx );
		}

		gfx->DrawIndexed( m_IB->GetCount(), 0u, 0u );
	}
	void Drawable::SetIndexBuffer( IndexBuffer* indexBuffer )
	{
		m_IB = indexBuffer;
	}
}