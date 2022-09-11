#include "Renderer.h"

#include "../Graphics/Graphics.h"
#include "../Drawable/Drawable.h"

namespace Katame
{
	Renderer::Renderer( Graphics* gfx )
		: gfx( gfx )
	{}

	Renderer::~Renderer()
	{
	}

	void Renderer::Submit( Drawable* drawable )
	{
		m_Drawables.push_back( drawable );
	}

	void Renderer::Execute()
	{
		for ( Drawable* drawable : m_Drawables )
		{
			drawable->Draw( gfx );
		}
	}
}