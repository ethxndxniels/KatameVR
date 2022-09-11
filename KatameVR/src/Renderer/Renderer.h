#pragma once

#include <vector>

namespace Katame
{
	class Graphics;
	class Drawable;

	class Renderer
	{
	public:
		Renderer( Graphics* gfx );
		~Renderer();
	public:
		void Submit( Drawable* drawable );
		void Execute();
	private:
		Graphics* gfx;
		std::vector<Drawable*> m_Drawables;
	};
}