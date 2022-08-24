#pragma once
#include "../Graphics/Graphics.h"
#include <DirectXMath.h>
#include <vector>
#include <memory>

namespace Katame
{
	namespace Bind
	{
		class Bindable;
		class IndexBuffer;
	}

	class Drawable
	{
	public:
		Drawable() = default;
		Drawable( const Drawable& ) = delete;
		virtual DirectX::XMMATRIX GetTransformXM() const noexcept = 0;
		void Draw( Graphics* gfx ) const noexcept;
		void DrawDepth( Graphics* gfx ) const noexcept;
		virtual ~Drawable() = default;
		template<class T>
		T* QueryBindable() noexcept
		{
			for (auto& pb : binds)
			{
				if (auto pt = dynamic_cast<T*>(pb.get()))
				{
					return pt;
				}
			}
			return nullptr;
		}
	protected:
		void AddBind( std::shared_ptr<Bind::Bindable> bind ) noexcept;
		const Bind::IndexBuffer* pIndexBuffer = nullptr;
		std::vector<std::shared_ptr<Bind::Bindable>> binds;
	};

}