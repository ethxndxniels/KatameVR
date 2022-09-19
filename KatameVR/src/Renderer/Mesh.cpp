#include "Mesh.h"
#include "../Core/Log.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

#include "../Graphics/Graphics.h"

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


		std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayout =
		{
			{"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Tangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Bitangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Texcoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	
		vs = new VertexShader( gfx, ".\\Shaders\\Bin\\PhongVS.cso" );
		ps = new PixelShader( gfx, ".\\Shaders\\Bin\\PhongPS.cso" );

		m_VertexBuffer = new VertexBuffer( gfx, m_Vertices.data(), (unsigned int)(m_Vertices.size() * sizeof( Vertex )), sizeof( Vertex ) );
		m_InputLayout = new InputLayout( gfx, inputLayout, *vs );

		// Extract indices from model
		m_Indices.reserve( mesh->mNumFaces );
		for (size_t i = 0; i < m_Indices.capacity(); i++)
		{
			KM_CORE_ASSERT( mesh->mFaces[i].mNumIndices == 3, "Must have 3 indices." );
			m_Indices.push_back( { mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] } );
		}

		m_IndexBuffer = new IndexBuffer( gfx, m_Indices.data(), (unsigned int)(m_Indices.size() * sizeof( Index )), sizeof( Index ) );
	}

	Mesh::~Mesh()
	{
	}

	void Mesh::Render( Graphics* gfx )
	{
		// TODO: Sort this out
		m_VertexBuffer->Bind( gfx );
		m_IndexBuffer->Bind( gfx ); 
		m_InputLayout->Bind( gfx );
		vs->Bind( gfx );
		ps->Bind( gfx );
		gfx->DrawIndexed( m_IndexBuffer->GetCount(), 0u, 0u );
	}

	bool flag = true;
	void Mesh::Update( float dt )
	{
		if (Pose.position.y > 0.5f)
			flag = false;
		else if ( Pose.position.y < -0.5f)
			flag = true;

		if ( flag )
		{ 
			Pose.position.y += dt * 0.1f;
			Pose.orientation.w += dt * 0.1f;
		}
		else
		{
			Pose.position.y -= dt * 0.1f;
			Pose.orientation.w -= dt * 0.1f;

		}
	}

	DirectX::XMMATRIX Mesh::GetModelMatrix()
	{
		return DirectX::XMMatrixTranspose( DirectX::XMMatrixScaling( Scale.x, Scale.y, Scale.z ) * LoadXrPose( Pose ) );
	}

}