#pragma once
#include "Bindable.h"
#include "BufferResource.h"
#include "../Renderer/Surface.h"

class Graphics;

namespace Katame
{
	class RenderTarget;

	class DepthStencil : public Bindable, public BufferResource
	{
		friend RenderTarget;
	public:
		void BindAsBuffer(Graphics& gfx) noexcept  override;
		void BindAsBuffer(Graphics& gfx, BufferResource* renderTarget) noexcept override;
		void BindAsBuffer(Graphics& gfx, RenderTarget* rt) noexcept;
		void Clear(Graphics& gfx) noexcept  override;
	protected:
		DepthStencil(Graphics& gfx, UINT width, UINT height, bool canBindShaderInput);
		ID3D11DepthStencilView* pDepthStencilView = nullptr;
	};

	class ShaderInputDepthStencil : public DepthStencil
	{
	public:
		ShaderInputDepthStencil(Graphics& gfx, UINT slot);
		ShaderInputDepthStencil(Graphics& gfx, UINT width, UINT height, UINT slot);
		void Bind(Graphics& gfx) noexcept  override;
	private:
		UINT slot;
		ID3D11ShaderResourceView* pShaderResourceView = nullptr;
	};

	class OutputOnlyDepthStencil : public DepthStencil
	{
	public:
		OutputOnlyDepthStencil(Graphics& gfx);
		OutputOnlyDepthStencil(Graphics& gfx, UINT width, UINT height);
		void Bind(Graphics& gfx) noexcept  override;
	};
}