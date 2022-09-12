#pragma once

#include <vector>

namespace Katame
{
	class Graphics;
	class Drawable;
	class VCBuffer;

	class Renderer
	{
	public:
		Renderer( Graphics* gfx );
		~Renderer();
	public:
		void Submit( Drawable& drawable );
		void Execute( VCBuffer* modelCBuf );
	private:
		Graphics* gfx;
		std::vector<Drawable*> m_Drawables;
	};
}