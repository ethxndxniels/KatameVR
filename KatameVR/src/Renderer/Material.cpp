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

		if (material.GetTexture( aiTextureType_DIFFUSE, 0, &texFileName ) == aiReturn_SUCCESS)
		{
			m_DiffTex = new Texture( gfx, rootPath + texFileName.C_Str() );
			shaderCode += "Dif";
		}
		if (material.GetTexture( aiTextureType_SPECULAR, 0, &texFileName ) == aiReturn_SUCCESS)
		{
			m_SpecTex = new Texture( gfx, rootPath + texFileName.C_Str(), 1 );
			shaderCode += "Spc";
		}
		if (material.GetTexture( aiTextureType_NORMALS, 0, &texFileName ) == aiReturn_SUCCESS)
		{
			m_NormTex = new Texture( gfx, rootPath + texFileName.C_Str(), 2 );
			shaderCode += "Nrm";
		}

		m_VS = new VertexShader( gfx, shaderCode + "VS.cso" );
		m_PS = new PixelShader( gfx, shaderCode + "PS.cso" );

		std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayout =
		{
			{"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Tangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Bitangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Texcoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		m_InputLayout = new InputLayout( gfx, inputLayout, *m_VS );
		m_Topology = new Topology( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		m_Rasterizer = new Rasterizer( gfx, true );
		m_Blender = new Blender( gfx, true );
		m_Sampler = new Sampler( gfx, Sampler::Type::Anisotropic, true );
	}
	void Material::Bind( Graphics* gfx )
	{
		m_InputLayout->Bind( gfx );
		if ( m_DiffTex ) m_DiffTex->Bind( gfx );
		if ( m_SpecTex ) m_SpecTex->Bind( gfx );
		if ( m_NormTex ) m_NormTex->Bind( gfx );
		m_VS->Bind( gfx );
		m_PS->Bind( gfx );
		m_Topology->Bind( gfx );
		m_Rasterizer->Bind( gfx );
		m_Blender->Bind( gfx );
		m_Sampler->Bind( gfx );
	}
}