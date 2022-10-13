#pragma once

#include "Bindable.h"

#include <d3d11.h>

#include <string>

namespace Katame
{
	class Graphics;

	class Texture : public Bindable
	{
	public:
		Texture( Graphics* gfx, const std::string& path, UINT slot = 0 );
		void Bind( Graphics* gfx ) override;
		std::string GetPath() { return path; };
	private:
		unsigned int slot;
	public:
		bool hasAlpha = false;
		std::string path;
		ID3D11ShaderResourceView* pTextureView;
	};
}
