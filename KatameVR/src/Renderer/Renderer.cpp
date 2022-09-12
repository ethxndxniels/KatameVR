#include "Renderer.h"

#include "../Graphics/Graphics.h"
#include "../Drawable/Drawable.h"
#include "../Bindables/Buffer.h"
#include "../Graphics/D3DCommon.h"

namespace Katame
{
	Renderer::Renderer( Graphics* gfx )
		: gfx( gfx )
	{}

	Renderer::~Renderer()
	{
	}

	void Renderer::Submit( Drawable& drawable )
	{
		m_Drawables.push_back( &drawable );
	}

	void Renderer::Execute( VCBuffer* modelCBuf )
	{
		ModelConstantBuffer model;
		for ( Drawable* drawable : m_Drawables )
		{	
			XMStoreFloat4x4( &model.Model, drawable->GetModelMatrix() );
			modelCBuf->Update( gfx, &model );
			modelCBuf->Bind( gfx );
			drawable->Draw( gfx );
		}
		m_Drawables.clear();
	}
}