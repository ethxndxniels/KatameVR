#pragma once
#include "../Graphics/Graphics.h"
#include <string>
#include <memory>

namespace Katame
{

	namespace Bind
	{
		class Bindable
		{
		public:
			virtual void Bind( Graphics* gfx ) noexcept = 0;
			virtual std::string GetUID() const noexcept
			{
				assert( false );
				return "";
			}
			virtual ~Bindable() = default;
		};
	}

}