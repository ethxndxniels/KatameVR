#include "TransformCbuf.h"

namespace Katame
{

	namespace Bind
	{
		TransformCbuf::TransformCbuf( Graphics* gfx, const Drawable& parent, UINT slot )
			:
			m_DrawableParent( parent )
		{
			if (!m_pVcbuf)
			{
				m_pVcbuf = std::make_unique<VertexConstantBuffer<Transforms>>( gfx, slot );
			}
		}

		void TransformCbuf::Bind( Graphics* gfx ) noexcept
		{
			UpdateBindImpl( gfx, GetTransforms( gfx ) );
		}

		void TransformCbuf::UpdateBindImpl( Graphics* gfx, const Transforms& tf ) noexcept
		{
			m_pVcbuf->Update( gfx, tf );
			m_pVcbuf->Bind( gfx );
		}

		TransformCbuf::Transforms TransformCbuf::GetTransforms( Graphics* gfx ) noexcept
		{
			return {};
			//// TODO: double check xrmatrix
			//const auto modelView = m_DrawableParent.GetTransformXM() * gfx->GetXRProjection();

			//return {
			//	DirectX::XMMatrixTranspose( m_DrawableParent.GetTransformXM() ),
			//	DirectX::XMMatrixTranspose( modelView ),
			//	DirectX::XMMatrixTranspose(
			//		modelView *
			//		gdi.GetProjMatrix()
			//	)
			//};
		}

		std::unique_ptr<VertexConstantBuffer<TransformCbuf::Transforms>> TransformCbuf::m_pVcbuf;
	}

}