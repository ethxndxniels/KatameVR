#pragma once

#include <vector>
#include <directxmath.h>
#include "../Bindables/CBuffer.h"

namespace Katame
{
	class Graphics;
	class Drawable;
	class Mesh;
	class Model;
	class OutputOnlyRenderTarget;
	class PointLight;

	class Renderer
	{
	public:
		Renderer( Graphics& gfx );
		~Renderer();
	public:
		void Submit( Drawable& drawable );
		void Submit( Model& model );
		void Submit( PointLight& pointLight );
		void Execute();
		void Draw();
		void Clear();
	public:
		void SetMainRenderTarget( OutputOnlyRenderTarget& renderTarget );
		void UpdateViewProjCBuf( DirectX::XMFLOAT4X4& viewProjection );
	private:
		Graphics& gfx;
		OutputOnlyRenderTarget* m_MainRenderTarget = nullptr;
		VCBuffer m_ModelCBuf;
		VCBuffer m_ViewProjCBuf;

		std::vector<Drawable*> m_Drawables;
		std::vector<Model*> m_Models;
		std::vector<PointLight*> m_PointLights;
	};
}