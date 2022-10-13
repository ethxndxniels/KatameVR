#include "Mesh.h"
#include "../Core/Log.h"

#include "../Graphics/Graphics.h"
#include <filesystem>

namespace Katame {

	namespace
	{
		const unsigned int ImportFlags =
			aiProcess_CalcTangentSpace | // calculate tangents and bitangents if possible
			aiProcess_JoinIdenticalVertices | // join identical vertices/ optimize indexing
			//aiProcess_ValidateDataStructure  | // perform a full validation of the loader's output
			aiProcess_Triangulate | // Ensure all verticies are triangulated (each 3 vertices are triangle)
			aiProcess_ConvertToLeftHanded | // convert everything to D3D left handed space (by default right-handed, for OpenGL)
			aiProcess_SortByPType | // ?
			aiProcess_ImproveCacheLocality | // improve the cache locality of the output vertices
			aiProcess_RemoveRedundantMaterials | // remove redundant materials
			aiProcess_FindDegenerates | // remove degenerated polygons from the import
			aiProcess_FindInvalidData | // detect invalid model data, such as invalid normal vectors
			aiProcess_GenUVCoords | // convert spherical, cylindrical, box and planar mapping to proper UVs
			aiProcess_TransformUVCoords | // preprocess UV transformations (scaling, translation ...)
			aiProcess_FindInstances | // search for instanced meshes and remove them by references to one master
			aiProcess_LimitBoneWeights | // limit bone weights to 4 per vertex
			aiProcess_OptimizeMeshes | // join small meshes, if possible;
			aiProcess_PreTransformVertices | //-- fixes the transformation issue.
			aiProcess_SplitByBoneCount | // split meshes with too many bones. Necessary for our (limited) hardware skinning shader
			0;
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
			{"Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Tangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Bitangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Texcoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	
		vs = new VertexShader( gfx, ".\\Shaders\\Bin\\PhongVS.cso" );
		ps = new PixelShader( gfx, ".\\Shaders\\Bin\\PhongPS.cso" );

		m_VertexBuffer = new VertexBuffer( gfx, m_Vertices.data(), (unsigned int)(m_Vertices.size() * sizeof( Vertex )), sizeof( Vertex ) );
		m_InputLayout = new InputLayout( gfx, inputLayout, *vs );

		// Extract indices from model
		//m_Indices.reserve( mesh->mNumFaces * 3 );
		for (size_t i = 0; i < mesh->mNumFaces; i++)
		{
			KM_CORE_ASSERT( mesh->mFaces[i].mNumIndices == 3, "Must have 3 indices." );
			for (size_t j = 0; j < mesh->mFaces[i].mNumIndices; j++ )
			{
				m_Indices.push_back( mesh->mFaces[i].mIndices[j] );
			}
		}

		m_IndexBuffer = new IndexBuffer( gfx, m_Indices.data(), (unsigned int)(m_Indices.size() * sizeof( unsigned int )), sizeof( unsigned int ) );
		
		if (mesh->mMaterialIndex >= 0) 
		{
			aiString texFileName;
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			const auto rootPath = std::filesystem::path(filename).parent_path().string() + "\\";

			if (material->GetTexture( aiTextureType_DIFFUSE, 0, &texFileName ) == aiReturn_SUCCESS)
			{
				m_DiffTex = new Texture( gfx, rootPath + texFileName.C_Str() );
			}
		}
		
		m_Topology = new Topology( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		m_Rasterizer = new Rasterizer( gfx, true );
		m_Blender = new Blender( gfx, false );
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
		m_Topology->Bind( gfx );
		m_Rasterizer->Bind( gfx );
		m_Blender->Bind( gfx );

		m_DiffTex->Bind( gfx );
		

		gfx->DrawIndexed( m_IndexBuffer->GetCount(), 0u, 0u );
	}

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

	void Mesh::SetData( XrPosef pose, XrVector3f scale )
	{
		Pose = pose;
		Scale = scale;
	}

	DirectX::XMMATRIX Mesh::GetModelMatrix()
	{
		return DirectX::XMMatrixTranspose( DirectX::XMMatrixScaling( Scale.x, Scale.y, Scale.z ) * LoadXrPose( Pose ) );
	}

}