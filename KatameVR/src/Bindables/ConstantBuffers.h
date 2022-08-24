#pragma once
#include "Bindable.h"
#include "BindableCodex.h"

namespace Katame
{

	namespace Bind
	{
		template<typename C>
		class ConstantBuffer : public Bindable
		{
		public:
			void Update( Graphics* gfx, const C& consts )
			{
				D3D11_MAPPED_SUBRESOURCE msr;
				gfx->m_Context->Map(
					m_pConstantBuffer, 0u,
					D3D11_MAP_WRITE_DISCARD, 0u,
					&msr
				);
				memcpy( msr.pData, &consts, sizeof( consts ) );
				gfx->m_Context->Unmap( m_pConstantBuffer, 0u );
			}
			ConstantBuffer( Graphics* gfx, const C& consts, UINT slot = 0u )
				:
				m_iSlot( slot )
			{
				D3D11_BUFFER_DESC cbd;
				cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				cbd.Usage = D3D11_USAGE_DYNAMIC;
				cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				cbd.MiscFlags = 0u;
				cbd.ByteWidth = sizeof( C );
				cbd.StructureByteStride = 0u;

				D3D11_SUBRESOURCE_DATA csd = {};
				csd.pSysMem = &consts;
				gfx->m_Device->CreateBuffer( &cbd, &csd, &m_pConstantBuffer );
			}
			ConstantBuffer( Graphics* gfx, UINT slot = 0u )
				:
				m_iSlot( slot )
			{
				D3D11_BUFFER_DESC cbd;
				cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				cbd.Usage = D3D11_USAGE_DYNAMIC;
				cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				cbd.MiscFlags = 0u;
				cbd.ByteWidth = sizeof( C );
				cbd.StructureByteStride = 0u;
				gfx->m_Device->CreateBuffer( &cbd, nullptr, &m_pConstantBuffer );
			}
		protected:
			ID3D11Buffer* m_pConstantBuffer;
			UINT m_iSlot;
		};

		template<typename C>
		class VertexConstantBuffer : public ConstantBuffer<C>
		{
			using ConstantBuffer<C>::m_pConstantBuffer;
			using ConstantBuffer<C>::m_iSlot;
		public:
			using ConstantBuffer<C>::ConstantBuffer;
			void Bind( Graphics* gfx ) noexcept override
			{
				gfx->m_Context->VSSetConstantBuffers( m_iSlot, 1u, &m_pConstantBuffer );
			}
			static std::shared_ptr<VertexConstantBuffer> Resolve( Graphics* gfx, const C& consts, UINT slot = 0 )
			{
				return Codex::Resolve<VertexConstantBuffer>( gfx, consts, slot );
			}
			static std::shared_ptr<VertexConstantBuffer> Resolve( Graphics* gfx, UINT slot = 0 )
			{
				return Codex::Resolve<VertexConstantBuffer>( gfx, slot );
			}
			static std::string GenerateUID( const C&, UINT slot )
			{
				return GenerateUID( slot );
			}
			static std::string GenerateUID( UINT slot = 0 )
			{
				using namespace std::string_literals;
				return typeid(VertexConstantBuffer).name() + "#"s + std::to_string( slot );
			}
			std::string GetUID() const noexcept override
			{
				return GenerateUID( m_iSlot );
			}
		};

		template<typename C>
		class PixelConstantBuffer : public ConstantBuffer<C>
		{
			using ConstantBuffer<C>::m_pConstantBuffer;
			using ConstantBuffer<C>::m_iSlot;
		public:
			using ConstantBuffer<C>::ConstantBuffer;
			void Bind( Graphics* gfx ) noexcept override
			{
				gfx->m_Context->PSSetConstantBuffers( m_iSlot, 1u, &m_pConstantBuffer );
			}
			static std::shared_ptr<PixelConstantBuffer> Resolve( Graphics* gfx, const C& consts, UINT slot = 0 )
			{
				return Codex::Resolve<PixelConstantBuffer>( gfx, consts, slot );
			}
			static std::shared_ptr<PixelConstantBuffer> Resolve( Graphics* gfx, UINT slot = 0 )
			{
				return Codex::Resolve<PixelConstantBuffer>( gfx, slot );
			}
			static std::string GenerateUID( const C&, UINT slot )
			{
				return GenerateUID( slot );
			}
			static std::string GenerateUID( UINT slot = 0 )
			{
				return typeid(PixelConstantBuffer).name();
			}
			std::string GetUID() const noexcept override
			{
				return GenerateUID( m_iSlot );
			}
		};
	}

}