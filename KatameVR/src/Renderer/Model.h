#pragma once

#include "../Graphics/Graphics.h"
#include <string>
#include <memory>
#include <filesystem>

struct aiMesh;
struct aiMaterial;
struct aiNode;

namespace Katame
{
	class Node;
	class Mesh;
	class VCBuffer;

	class Model
	{
	public:
		Model( Graphics* gfx, const std::string& pathString, float scale = 1.0f );
		~Model() noexcept;
	public:
		void Render( Graphics* gfx, VCBuffer* modelCBuf );
		void Update( float dt );
		void SetData( XrPosef pose, XrVector3f scale );
		DirectX::XMMATRIX GetModelMatrix();
		std::vector<Mesh*> GetMeshes();
	private:
	//	static std::unique_ptr<Mesh> ParseMesh( Graphics* gfx, const aiMesh& mesh, const aiMaterial* const* pMaterials, const std::filesystem::path& path, float scale );
		std::unique_ptr<Node> ParseNode( int& nextId, const aiNode& node, float scale ) noexcept;
	private:
		std::unique_ptr<Node> pRoot;
		// sharing meshes here perhaps dangerous?
		std::vector<Mesh*> meshPtrs;
		XrPosef m_Pose = { { 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
		XrVector3f m_Scale = { 0.025f, 0.025f, 0.025f };
	};
}