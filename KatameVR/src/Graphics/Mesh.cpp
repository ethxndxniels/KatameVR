#include "Mesh.h"
#include "../Core/Log.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

#define KM_CORE_ASSERT(x, ...) { if(!(x)) { KM_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }

namespace Katame {

	namespace {
		const unsigned int ImportFlags =
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_SortByPType |
			aiProcess_PreTransformVertices |
			aiProcess_GenNormals |
			aiProcess_GenUVCoords |
			aiProcess_OptimizeMeshes |
			aiProcess_Debone |
			aiProcess_ValidateDataStructure;
	}

	struct LogStream : public Assimp::LogStream
	{
		static void Initialize()
		{
			if (Assimp::DefaultLogger::isNullLogger())
			{
				Assimp::DefaultLogger::create( "", Assimp::Logger::VERBOSE );
				Assimp::DefaultLogger::get()->attachStream( new LogStream, Assimp::Logger::Err | Assimp::Logger::Warn );
			}
		}

		void write( const char* message ) override
		{
			KM_CORE_ERROR( "Assimp error: {0}", message );
		}
	};

	Mesh::Mesh( const std::string& filename, Graphics* gfx )
		: m_FilePath( filename )
	{
		LogStream::Initialize();

		KM_CORE_INFO( "Loading mesh: {0}", filename.c_str() );

		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile( filename, ImportFlags );
		if (!scene || !scene->HasMeshes())
			KM_CORE_ERROR( "Failed to load mesh file: {0}", filename );

		aiMesh* mesh = scene->mMeshes[0];

		KM_CORE_ASSERT( mesh->HasPositions(), "Meshes require positions." );
		KM_CORE_ASSERT( mesh->HasNormals(), "Meshes require normals." );

		m_Vertices.reserve( mesh->mNumVertices );

		// Extract vertices from model
		for (size_t i = 0; i < m_Vertices.capacity(); i++)
		{
			Vertex vertex;
			vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
			vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

			if (mesh->HasTangentsAndBitangents())
			{
				vertex.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
				vertex.Binormal = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
			}

			if (mesh->HasTextureCoords( 0 ))
				vertex.Texcoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
			m_Vertices.push_back( vertex );
		}

		Dvtx::VertexBuffer vbuf( std::move(
			Dvtx::VertexLayout{}
			.Append( Dvtx::VertexLayout::Position3D )
			.Append( Dvtx::VertexLayout::Normal )
			.Append( Dvtx::VertexLayout::Tangent )
			.Append( Dvtx::VertexLayout::Bitangent )
			.Append( Dvtx::VertexLayout::Texture2D )
		) );

		m_VertexBuffer = new Bind::VertexBuffer( gfx, "test", vbuf );

		// Extract indices from model
		m_Indices.reserve( mesh->mNumFaces );
		for (size_t i = 0; i < m_Indices.capacity(); i++)
		{
			KM_CORE_ASSERT( mesh->mFaces[i].mNumIndices == 3, "Must have 3 indices." );
			m_Indices.push_back( { mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] } );
		}

		m_IndexBuffer = new Bind::IndexBuffer( gfx, m_Indices.data(), m_Indices.size() * sizeof( Index ) );
	}

	Mesh::~Mesh()
	{
	}

	void Mesh::Render( Graphics* gfx )
	{
		// TODO: Sort this out
		m_VertexBuffer->Bind( gfx );
		m_IndexBuffer->Bind( gfx );
		gfx->DrawIndexed( m_IndexBuffer->GetCount() );
	}

}
