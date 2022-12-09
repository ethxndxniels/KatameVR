#pragma once
#include "Bindable.h"
#include "BufferResource.h"
#include <array>
#include <optional>
#include <wtypes.h>
#include <d3d11.h>
#include <string>

namespace Katame
{
	class DepthStencil;
	class Graphics;
	class Surface;

	class RenderTarget : public Bindable, public BufferResource
	{
	public:
		void BindAsBuffer(Graphics* gfx) noexcept override;
		void BindAsBuffer(Graphics* gfx, BufferResource* depthStencil) noexcept override;
		void BindAsBuffer(Graphics* gfx, DepthStencil* depthStencil) noexcept;
		void Clear(Graphics* gfx) noexcept override;
		void Clear(Graphics* gfx, const std::array<float, 4>& color) noexcept;
		UINT GetWidth() const noexcept;
		UINT GetHeight() const noexcept;
		void BindAsBuffer(Graphics* gfx, ID3D11DepthStencilView* pDepthStencilView) noexcept;
	protected:
		RenderTarget(Graphics* gfx, ID3D11Texture2D* pTexture);
		RenderTarget(Graphics* gfx, UINT width, UINT height);
		UINT width;
		UINT height;
		ID3D11RenderTargetView* pTargetView;
	};

	class ShaderInputRenderTarget : public RenderTarget
	{
	public:
		ShaderInputRenderTarget(Graphics* gfx, UINT width, UINT height, UINT slot);
		void Bind(Graphics* gfx) override;
		Surface ToSurface(Graphics* gfx) const;
	private:
		UINT slot;
		ID3D11ShaderResourceView* pShaderResourceView;
	};

	class OutputOnlyRenderTarget : public RenderTarget
	{
	public:
		void Bind(Graphics* gfx) override;
		OutputOnlyRenderTarget(Graphics* gfx, ID3D11Texture2D* pTexture );
	};
}