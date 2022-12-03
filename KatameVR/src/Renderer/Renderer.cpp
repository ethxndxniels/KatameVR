#include "Renderer.h"

#include "../Graphics/Graphics.h"
#include "../Graphics/D3DCommon.h"
#include "../Bindables/Buffer.h"
#include "../Drawable/Drawable.h"
#include "../Renderer/Model.h"

#include "../../vendor/imgui/imgui.h"

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
			modelCBuf->Bind( gfx );
			drawable->Render( gfx );
		}

		for ( Model* draw_model : m_Models )
		{
			XMStoreFloat4x4(&model.Model, draw_model->GetModelMatrix());
			modelCBuf->Update( gfx, &model );
			modelCBuf->Bind( gfx) ;
			draw_model->Render( gfx );
		}

		ImGui::ShowDemoWindow();
	}

	void Renderer::Clear()
	{
		m_Drawables.clear();
		m_Models.clear();

		ID3D11ShaderResourceView* const pNullTex = nullptr;
		gfx->m_Context->PSSetShaderResources( 0u, 1u, &pNullTex );
	}
}