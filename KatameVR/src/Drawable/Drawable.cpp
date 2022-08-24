#include "Drawable.h"
#include "../Bindables/IndexBuffer.h"
#include <cassert>

namespace Katame
{
	using namespace Bind;

	void Drawable::Draw( Graphics* gfx ) const noexcept
	{
		// bindables
		for (auto& b : binds)
		{
			b->Bind( gfx );
		}

		// draw
		gfx->DrawIndexed( pIndexBuffer->GetCount() );
	}

	void Drawable::DrawDepth( Graphics* gfx ) const noexcept
	{
		// bindables
		for (auto& b : binds)
		{
			b->Bind( gfx );
		}

		gfx->m_Context->PSSetShader( nullptr, nullptr, 0u );

		// draw
		gfx->DrawIndexed( pIndexBuffer->GetCount() );
	}

	void Drawable::AddBind( std::shared_ptr<Bind::Bindable> bind ) noexcept
	{
		if (typeid(*bind) == typeid(IndexBuffer))
		{
			assert( "Binding multiple index buffers not allowed" && pIndexBuffer == nullptr );
			pIndexBuffer = &static_cast<IndexBuffer&>(*bind);
		}
		binds.push_back( std::move( bind ) );
	}

}