#pragma once

#include "../Graphics/Graphics.h"
#include "../Drawable/Drawable.h"

class Material;
struct aiMesh;

namespace Katame
{
	class Mesh : public Drawable
	{
		struct Vertex
		{
			DirectX::XMFLOAT3 Position;
			DirectX::XMFLOAT3 Normal;
			DirectX::XMFLOAT3 Tangent;
			DirectX::XMFLOAT3 Binormal;
			DirectX::XMFLOAT2 Texcoord;
		};
		static_assert(sizeof( Vertex ) == 14 * sizeof( float ));
	public:
		Mesh( Graphics* gfx, const Material& mat, const aiMesh& mesh, float scale = 1.0f );
		~Mesh();
		DirectX::XMMATRIX GetModelMatrix() override;
	private:
		std::vector<Vertex> m_Vertices;
		std::vector<unsigned int> m_Indices;

		IndexBuffer* m_IndexBuffer;
		mutable DirectX::XMFLOAT4X4 transform;
	};
}