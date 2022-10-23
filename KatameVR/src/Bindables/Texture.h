#pragma once

#include "Bindable.h"

#include "../Renderer/Surface.h"

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
		void Save();
	private:
		unsigned int slot;
	public:
		Surface m_Surface;
		bool hasAlpha = false;
		std::string path;
		ID3D11ShaderResourceView* pTextureView;
	};
}
