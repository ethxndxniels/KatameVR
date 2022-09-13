#include "Renderer.h"

#include "../Graphics/Graphics.h"
#include "../Drawable/Drawable.h"
#include "Mesh.h"
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

	void Renderer::Submit( Mesh& mesh )
	{
		m_Meshes.push_back( &mesh );
	}

	void Renderer::Execute( VCBuffer* modelCBuf )
	{
		ModelConstantBuffer model;
		for ( Drawable* drawable : m_Drawables )
		{	
			XMStoreFloat4x4( &model.Model, drawable->GetModelMatrix() );
			modelCBuf->Update( gfx, &model );
			drawable->Render( gfx );
		}

		for ( Mesh* mesh : m_Meshes )
		{
			XMStoreFloat4x4( &model.Model, mesh->GetModelMatrix() );
			modelCBuf->Update( gfx, &model );
			mesh->Render( gfx );
		}
	}

	void Renderer::Clear()
	{
		m_Drawables.clear();
	}
}