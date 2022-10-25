#include "Material.h"

#include <filesystem>

#include <assimp/types.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../Bindables/Texture.h"
#include "../Bindables/Sampler.h"

namespace Katame
{
	Material::Material( Graphics* gfx, const aiMaterial& material, const std::filesystem::path& path ) noexcept
		:
		modelPath( path.string() )
	{
		const auto rootPath = path.parent_path().string() + "\\";
		{
			aiString tempName;
			material.Get( AI_MATKEY_NAME, tempName );
			name = tempName.C_Str();
		}

		std::string shaderCode = "./Shaders/Bin/Phong";
		aiString texFileName;

		// diffuse
			
		if (material.GetTexture( aiTextureType_DIFFUSE, 0, &texFileName ) == aiReturn_SUCCESS)
		{
			new Texture( gfx, rootPath + texFileName.C_Str() );
		}
		if (material.GetTexture( aiTextureType_SPECULAR, 0, &texFileName ) == aiReturn_SUCCESS)
		{
			new Texture( gfx, rootPath + texFileName.C_Str(), 1 );
		}
		if (material.GetTexture( aiTextureType_NORMALS, 0, &texFileName ) == aiReturn_SUCCESS)
		{
			new Texture( gfx, rootPath + texFileName.C_Str(), 2 );
		}
		//step.AddBindable( std::make_shared<TransformCbuf>( gfx, 0u ) );
		new VertexShader( gfx, shaderCode + "_VS.cso" );
		//new InputLayout( gfx, vtxLayout, *pvs ) );
		new PixelShader( gfx, shaderCode + "_PS.cso" );
		
		new Sampler( gfx ) );
	}

	VertexBuffer Material::ExtractVertices( const aiMesh& mesh ) const noexcept
	{
		return { vtxLayout,mesh };
	}
	std::vector<unsigned short> Material::ExtractIndices( const aiMesh& mesh ) const noexcept
	{
		std::vector<unsigned short> indices;
		indices.reserve( mesh.mNumFaces * 3 );
		for (unsigned int i = 0; i < mesh.mNumFaces; i++)
		{
			const auto& face = mesh.mFaces[i];
			assert( face.mNumIndices == 3 );
			indices.push_back( face.mIndices[0] );
			indices.push_back( face.mIndices[1] );
			indices.push_back( face.mIndices[2] );
		}
		return indices;
	}
	std::shared_ptr<Bind::VertexBuffer> Material::MakeVertexBindable( GraphicsDeviceInterface& gfx, const aiMesh& mesh, float scale ) const noexcept
	{
		auto vtc = ExtractVertices( mesh );
		if (scale != 1.0f)
		{
			for (auto i = 0u; i < vtc.Size(); i++)
			{
				DirectX::XMFLOAT3& pos = vtc[i].Attr<Dvtx::VertexLayout::ElementType::Position3D>();
				pos.x *= scale;
				pos.y *= scale;
				pos.z *= scale;
			}
		}
		return Bind::VertexBuffer::Resolve( gfx, MakeMeshTag( mesh ), std::move( vtc ) );
	}
	std::shared_ptr<Bind::IndexBuffer> Material::MakeIndexBindable( GraphicsDeviceInterface& gfx, const aiMesh& mesh ) const noexcept
	{
		return Bind::IndexBuffer::Resolve( gfx, MakeMeshTag( mesh ), ExtractIndices( mesh ) );
	}
	std::string Material::MakeMeshTag( const aiMesh& mesh ) const noexcept
	{
		return modelPath + "%" + mesh.mName.C_Str();
	}
	std::vector<Technique> Material::GetTechniques() const noexcept
	{
		return techniques;
	}
}