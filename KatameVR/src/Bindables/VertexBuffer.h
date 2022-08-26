#pragma once

#include "Bindable.h"
#include "../Graphics/Vertex.h"
#include <memory>

namespace Katame
{
	namespace Bind
	{

		class VertexBuffer : public Bindable
		{
		public:
			VertexBuffer( Graphics* gfx, void* buffer, unsigned int size );
			VertexBuffer( Graphics* gfx, const std::string& tag, const Dvtx::VertexBuffer& vbuf );
			VertexBuffer( Graphics* gfx, const Dvtx::VertexBuffer& vbuf );
			void Bind( Graphics* gfx ) noexcept override;
			static std::shared_ptr<VertexBuffer> Resolve( Graphics* gfx, const std::string& tag,
				const Dvtx::VertexBuffer& vbuf );
			template<typename...Ignore>
			static std::string GenerateUID( const std::string& tag, Ignore&&...ignore )
			{
				return GenerateUID_( tag );
			}
			std::string GetUID() const noexcept override;
		private:
			static std::string GenerateUID_( const std::string& tag );
		protected:
			std::string m_sTag;
			UINT m_iStride;
			ID3D11Buffer* m_pVertexBuffer;
		};

	}
}