#pragma once

#include <vector>

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
		void Submit( Bindable* bind );
		void Draw( Graphics* gfx );
	protected:
		void SetIndexBuffer( IndexBuffer* indexBuffer );
	private:
		IndexBuffer* m_IB;
		std::vector<Bindable*> binds;
	};
}