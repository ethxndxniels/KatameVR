#pragma once

namespace Katame
{
	class Graphics;

	class Bindable
	{
	public:
		virtual void Bind( Graphics* gfx ) = 0;
	};
}