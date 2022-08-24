#pragma once
#include "ConstantBuffers.h"
#include "../Drawable/Drawable.h"
#include <DirectXMath.h>

namespace Katame
{

	namespace Bind
	{
		class TransformCbuf : public Bindable
		{
		protected:
			struct Transforms
			{
				DirectX::XMMATRIX m_ModelMatrix;
				DirectX::XMMATRIX m_ModelViewMatrix;
				DirectX::XMMATRIX m_ModelViewProjMatrix;
			};
		public:
			TransformCbuf( Graphics* gfx, const Drawable& parent, UINT slot = 0u );
			void Bind( Graphics* gfx ) noexcept override;
		protected:
			void UpdateBindImpl( Graphics* gfx, const Transforms& tf ) noexcept;
			Transforms GetTransforms( Graphics* gfx ) noexcept;
		private:
			static std::unique_ptr<VertexConstantBuffer<Transforms>> m_pVcbuf;
			const Drawable& m_DrawableParent;
		};
	}

}