#include "Mesh.h"
#include "../Core/Log.h"

#include "../Graphics/Graphics.h"
#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Katame 
{
	namespace dx = DirectX;

	// Mesh
	Mesh::Mesh( Graphics* gfx, const Material& mat, const aiMesh& mesh, float scale )
	{
		m_Vertices.reserve( mesh.mNumVertices );

		// Extract vertices from model
		for (size_t i = 0; i < m_Vertices.capacity(); i++)
		{
			Vertex vertex;
			vertex.Position = { mesh.mVertices[i].x, mesh.mVertices[i].y, mesh.mVertices[i].z };
			vertex.Normal = { mesh.mNormals[i].x, mesh.mNormals[i].y, mesh.mNormals[i].z };

			if (mesh.HasTangentsAndBitangents())
			{
				vertex.Tangent = { mesh.mTangents[i].x, mesh.mTangents[i].y, mesh.mTangents[i].z };
				vertex.Binormal = { mesh.mBitangents[i].x, mesh.mBitangents[i].y, mesh.mBitangents[i].z };
			}

			if (mesh.HasTextureCoords( 0 ))
				vertex.Texcoord = { mesh.mTextureCoords[0][i].x, mesh.mTextureCoords[0][i].y };

			m_Vertices.push_back( vertex );
		}

		AddBind( new VertexBuffer( gfx, m_Vertices.data(), (unsigned int)(m_Vertices.size() * sizeof( Vertex )), sizeof( Vertex ) ) );
	
		for (size_t i = 0; i < mesh.mNumFaces; i++)
		{
			KM_CORE_ASSERT( mesh.mFaces[i].mNumIndices == 3, "Must have 3 indices." );
			for (size_t j = 0; j < mesh.mFaces[i].mNumIndices; j++)
			{
				m_Indices.push_back( mesh.mFaces[i].mIndices[j] );
			}
		}

		SetIndexBuffer( new IndexBuffer( gfx, m_Indices.data(), (unsigned int)(m_Indices.size() * sizeof( unsigned int )), sizeof( unsigned int ) ) );
	}

	void Mesh::Render( Graphics* gfx, DirectX::FXMMATRIX accumulatedTranform )
	{
		dx::XMStoreFloat4x4( &transform, accumulatedTranform );
		Drawable::Render( gfx );
	}

	Mesh::~Mesh()
	{
	}

	DirectX::XMMATRIX Mesh::GetModelMatrix() 
	{
		return DirectX::XMLoadFloat4x4( &transform );
	}
}