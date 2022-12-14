#pragma once

#include <vector>
#include <DirectXMath.h>

namespace Katame
{
	class Graphics;
	class Bindable;

	class IndexBuffer;
	class VertexBuffer;
	class Topology;
	class InputLayout;

	class Drawable
	{
	public:
		Drawable() = default;
		virtual ~Drawable();
		void AddBind( Bindable* bind );
		void Render( Graphics& gfx );
		virtual DirectX::XMMATRIX GetModelMatrix() = 0;
	protected:
		void SetIndexBuffer( IndexBuffer* indexBuffer );
	private:
		IndexBuffer* m_IB;
		std::vector<Bindable*> binds;
	};
}