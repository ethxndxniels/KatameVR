#pragma once

#include "../Graphics/Graphics.h"
#include "../Drawable/Drawable.h"

class Material;
struct aiMesh;

namespace Katame
{
	class Mesh : public Drawable
	{
	public:
		Mesh( Graphics* gfx, const Material& mat, const aiMesh& mesh, float scale = 1.0f ) noexcept;
		~Mesh();
		DirectX::XMMATRIX GetModelMatrix() override;
	private:
		mutable DirectX::XMFLOAT4X4 transform;
	};
}