#include "Model.h"                                                                                                                                                                                                                                                                                                                                                       
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Node.h"
#include "Mesh.h"
#include "Material.h"
#include "../Utilities/KatameXM.h"
#include <assimp/DefaultLogger.hpp>

#include "../Core/Log.h"

namespace dx = DirectX;

namespace Katame
{
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

	Model::Model( Graphics* gfx, const std::string& pathString, const float scale )
	{
		LogStream::Initialize();

		KM_CORE_INFO( "Loading mesh: {0}", pathString.c_str() );

		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile( pathString, ImportFlags );
		if (!scene || !scene->HasMeshes())
			KM_CORE_ERROR( "Failed to load mesh file: {0}", pathString );

		aiMesh* mesh = scene->mMeshes[0];

		KM_CORE_ASSERT( mesh->HasPositions(), "Meshes require positions." );
		KM_CORE_ASSERT( mesh->HasNormals(), "Meshes require normals." );

		// parse materials
		std::vector<Material> materials;
		materials.reserve( scene->mNumMaterials );
		for (size_t i = 0; i < scene->mNumMaterials; i++)
		{
			materials.push_back( { gfx, *scene->mMaterials[i], pathString } );
		}

		for (size_t i = 0; i < scene->mNumMeshes; i++)
		{
			const auto& mesh = *scene->mMeshes[i];
			meshPtrs.push_back( new Mesh( gfx, materials[mesh.mMaterialIndex], mesh, scale ) );
		}

		int nextId = 0;
		pRoot = ParseNode( nextId, *scene->mRootNode, scale );
	}

	Model::~Model() noexcept
	{}

	void Model::Render( Graphics* gfx, VCBuffer* modelCBuf )
	{
		pRoot->Render( gfx, GetModelMatrix(), modelCBuf );
	}

	void Model::Update( float dt )
	{
		m_Pose.position.y -= 1.0f * dt;
	}

	void Model::SetData( XrPosef pose, XrVector3f scale )
	{
		m_Pose = pose;
		m_Scale = scale;
	}

	DirectX::XMMATRIX Model::GetModelMatrix()
	{
		return DirectX::XMMatrixTranspose( DirectX::XMMatrixScaling( m_Scale.x, m_Scale.y, m_Scale.z ) * LoadXrPose( m_Pose ) );
	}

	std::vector<Mesh*> Model::GetMeshes()
	{
		return meshPtrs;
	}

	std::unique_ptr<Node> Model::ParseNode( int& nextId, const aiNode& node, float scale ) noexcept
	{
		namespace dx = DirectX;
		const auto transform = ScaleTranslation( dx::XMMatrixTranspose( dx::XMLoadFloat4x4(
			reinterpret_cast<const dx::XMFLOAT4X4*>(&node.mTransformation)
		) ), scale );

		std::vector<Mesh*> curMeshPtrs;
		curMeshPtrs.reserve( node.mNumMeshes );
		for (size_t i = 0; i < node.mNumMeshes; i++)
		{
			const auto meshIdx = node.mMeshes[i];
			curMeshPtrs.push_back( meshPtrs[meshIdx] );
		}

		auto pNode = std::make_unique<Node>( nextId++, node.mName.C_Str(), std::move( curMeshPtrs ), transform );
		for (size_t i = 0; i < node.mNumChildren; i++)
		{
			pNode->AddChild( ParseNode( nextId, *node.mChildren[i], scale ) );
		}

		return pNode;
	}
}