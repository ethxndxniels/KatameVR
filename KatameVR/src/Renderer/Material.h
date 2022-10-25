#pragma once

#include "../Graphics/Graphics.h"

// Bindables
#include "../Bindables/Buffer.h"
#include "../Bindables/InputLayout.h"
#include "../Bindables/VertexShader.h"
#include "../Bindables/PixelShader.h"
#include "../Bindables/Texture.h"
#include "../Bindables/Topology.h"
#include "../Bindables/Rasterizer.h"
#include "../Bindables/Blender.h"
#include "../Bindables/Sampler.h"

#include <vector>
#include <filesystem>

struct aiMaterial;
struct aiMesh;

namespace Katame
{
	class VertexBuffer;
	class IndexBuffer;

	class Material
	{
	public:
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
		Material( Graphics* gfx, const aiMaterial& material, const std::filesystem::path& path ) noexcept;
		VertexBuffer ExtractVertices( const aiMesh& mesh ) const noexcept;
		std::vector<unsigned short> ExtractIndices( const aiMesh& mesh ) const noexcept;
		std::shared_ptr<VertexBuffer> MakeVertexBindable( Graphics* gfx, const aiMesh& mesh, float scale = 1.0f ) const noexcept;
		std::shared_ptr<IndexBuffer> MakeIndexBindable( Graphics* gfx, const aiMesh& mesh ) const noexcept;
	private:
		std::string MakeMeshTag( const aiMesh& mesh ) const noexcept;
	private:
		std::string modelPath;
		std::string name;
	};
}
