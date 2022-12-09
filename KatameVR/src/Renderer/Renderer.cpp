#include "Renderer.h"

#include "../Graphics/Graphics.h"
#include "../Graphics/D3DCommon.h"
#include "../Bindables/Buffer.h"
#include "../Bindables/DepthStencil.h"
#include "../Bindables/RenderTarget.h"
#include "../Drawable/Drawable.h"
#include "../Renderer/Model.h"
#include "../Graphics/D3DCommon.h"
#include "../Lights/PointLight.h"

#include "../../vendor/imgui/imgui.h"

namespace Katame
{
	Renderer::Renderer( Graphics& gfx )
		: 
		gfx( gfx ),
		m_ModelCBuf( gfx, 0u, sizeof(ModelConstantBuffer) ),
		m_ViewProjCBuf( gfx, 1u, sizeof(DirectX::XMFLOAT4X4) )
	{
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

	void Renderer::Submit( PointLight& pointLight )
	{
		m_PointLights.push_back( &pointLight );
		Submit( pointLight.GetLightCore() );
	}

	void Renderer::Execute()
	{
		// Light Data
		m_PointLights[0]->Bind();

		// Shadow Map Pass
		ShaderInputDepthStencil shadowMap{ gfx, (UINT)gfx.GetWidth(), (UINT)gfx.GetHeight(), 5 };
		shadowMap.Clear( gfx );
		shadowMap.BindAsBuffer( gfx );

		m_PointLights[0]->BindViewProj();

		Draw();

		// Forward Pass
		OutputOnlyDepthStencil depthStencil{ gfx, (UINT)gfx.GetWidth(), (UINT)gfx.GetHeight() };
		depthStencil.Clear( gfx );

		m_MainRenderTarget->Clear( gfx );
		m_MainRenderTarget->BindAsBuffer( gfx, &depthStencil );

		m_ViewProjCBuf.Bind( gfx );

		// Resources
		shadowMap.Bind( gfx );

		Draw();
	}

	void Renderer::Draw()
	{
		ModelConstantBuffer model;

		for (Drawable* drawable : m_Drawables)
		{
			XMStoreFloat4x4( &model.Model, drawable->GetModelMatrix() );
			m_ModelCBuf.Update( gfx, &model );
			m_ModelCBuf.Bind( gfx );
			drawable->Render( gfx );
		}

		for (Model* draw_model : m_Models)
		{
			XMStoreFloat4x4(  &model.Model, draw_model->GetModelMatrix() );
			m_ModelCBuf.Update( gfx, &model );
			m_ModelCBuf.Bind( gfx );
			draw_model->Render( gfx );
		}
	}

	void Renderer::Clear()
	{
		m_Drawables.clear();
		m_Models.clear();
		m_PointLights.clear();

		ID3D11ShaderResourceView* const pNullTex = nullptr;
		gfx.m_Context->PSSetShaderResources( 0u, 1u, &pNullTex );
	}

	void Renderer::SetMainRenderTarget(OutputOnlyRenderTarget& renderTarget)
	{
		m_MainRenderTarget = &renderTarget;
	}
	void Renderer::UpdateViewProjCBuf( DirectX::XMFLOAT4X4& viewProjection )
	{
		m_ViewProjCBuf.Update( gfx, &viewProjection );
	}
}