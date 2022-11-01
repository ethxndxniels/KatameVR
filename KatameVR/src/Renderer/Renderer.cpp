#include "Renderer.h"

#include "../Graphics/Graphics.h"
#include "../Drawable/Drawable.h"
#include "Mesh.h"
#include "../Bindables/Buffer.h"
#include "../Graphics/D3DCommon.h"
#include "../Renderer/Model.h"

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

	void Renderer::Submit( Model& model )
	{
		m_Models.push_back( &model );
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

		for ( Model* draw_model : m_Models )
		{
			XMStoreFloat4x4( &model.Model, draw_model->GetModelMatrix() );
			modelCBuf->Update( gfx, &model );
			draw_model->Render( gfx );
		}
	}

	void Renderer::Clear()
	{
		m_Drawables.clear();
		m_Meshes.clear();

		ID3D11ShaderResourceView* const pNullTex = nullptr;
		gfx->m_Context->PSSetShaderResources( 0u, 1u, &pNullTex );
	}
}