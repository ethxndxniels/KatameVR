#include "Renderer.h"

#include "../Graphics/Graphics.h"
#include "../Graphics/D3DCommon.h"
#include "../Bindables/Buffer.h"
#include "../Bindables/DepthStencil.h"
#include "../Bindables/RenderTarget.h"
#include "../Drawable/Drawable.h"
#include "../Renderer/Model.h"
#include "../Graphics/D3DCommon.h"

#include "../../vendor/imgui/imgui.h"

namespace Katame
{
	Renderer::Renderer( Graphics* gfx )
		: gfx( gfx )
	{

		m_ModelCBuf = new VCBuffer(gfx, 0u, sizeof(ModelConstantBuffer));
		m_ViewProjCBuf = new VCBuffer(gfx, 1u, sizeof(DirectX::XMFLOAT4X4));
	}

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

	void Renderer::Execute()
	{
		// Depth Pass
		// TODO: Add point light transform
		OutputOnlyDepthStencil* depthStencil = new OutputOnlyDepthStencil( gfx, gfx->GetWidth(), gfx->GetHeight() );
		depthStencil->Clear( gfx );

		m_MainRenderTarget->Clear( gfx );
		m_MainRenderTarget->BindAsBuffer( gfx, depthStencil );

		m_ModelCBuf->Bind( gfx );
		m_ViewProjCBuf->Bind( gfx );

		Draw( m_ModelCBuf );
	}

	void Renderer::Draw(VCBuffer* modelCBuf)
	{
		ModelConstantBuffer model;

		for (Drawable* drawable : m_Drawables)
		{
			XMStoreFloat4x4(&model.Model, drawable->GetModelMatrix());
			modelCBuf->Update(gfx, &model);
			modelCBuf->Bind(gfx);
			drawable->Render(gfx);
		}

		for (Model* draw_model : m_Models)
		{
			XMStoreFloat4x4(&model.Model, draw_model->GetModelMatrix());
			modelCBuf->Update(gfx, &model);
			modelCBuf->Bind(gfx);
			draw_model->Render(gfx);
		}
	}

	void Renderer::Clear()
	{
		m_Drawables.clear();
		m_Models.clear();

		ID3D11ShaderResourceView* const pNullTex = nullptr;
		gfx->m_Context->PSSetShaderResources( 0u, 1u, &pNullTex );
	}

	void Renderer::SetMainRenderTarget(OutputOnlyRenderTarget* renderTarget)
	{
		m_MainRenderTarget = renderTarget;
	}
	void Renderer::UpdateViewProjCBuf( DirectX::XMFLOAT4X4& viewProjection )
	{
		m_ViewProjCBuf->Update( gfx, &viewProjection );
	}
}