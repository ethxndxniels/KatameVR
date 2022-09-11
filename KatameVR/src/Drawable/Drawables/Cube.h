#pragma once

#include "../Drawable.h"

namespace Katame
{
	class Graphics;

	class Cube : public Drawable
	{
	public:
		Cube( Graphics* gfx );
		~Cube();
	};
}
