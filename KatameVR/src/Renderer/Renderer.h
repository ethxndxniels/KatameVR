#pragma once

#include <vector>
#include <directxmath.h>

namespace Katame
{
	class Graphics;
	class Drawable;
	class Mesh;
	class VCBuffer;
	class Model;
	class OutputOnlyRenderTarget;

	class Renderer
	{
	public:
		Renderer( Graphics* gfx );
		~Renderer();
	public:
		void Submit( Drawable& drawable );
		void Submit( Model& model );
		void Execute();
		void Draw( VCBuffer* modelCBuf );
		void Clear();
	public:
		void SetMainRenderTarget( OutputOnlyRenderTarget* renderTarget );
		void UpdateViewProjCBuf( DirectX::XMFLOAT4X4& viewProjection );
	private:
		Graphics* gfx;
		std::vector<Drawable*> m_Drawables;
		std::vector<Model*> m_Models;
		OutputOnlyRenderTarget* m_MainRenderTarget;

		VCBuffer* m_ModelCBuf;
		VCBuffer* m_ViewProjCBuf;
	};
}