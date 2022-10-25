#pragma once

#include "../Graphics/Graphics.h"
#include <string>
#include <memory>
#include <filesystem>

class Node;
class Mesh;
struct aiMesh;
struct aiMaterial;
struct aiNode;

namespace Katame
{
	class Model
	{
	public:
		Model( Graphics* gfx, const std::string& pathString, float scale = 1.0f );
		~Model() noexcept;
	public:
		void Render( Graphics* gfx );
		void Update( float dt );
		void SetData( XrPosef pose, XrVector3f scale );
	private:
		static std::unique_ptr<Mesh> ParseMesh( Graphics* gfx, const aiMesh& mesh, const aiMaterial* const* pMaterials, const std::filesystem::path& path, float scale );
		std::unique_ptr<Node> ParseNode( int& nextId, const aiNode& node, float scale ) noexcept;
	private:
		std::unique_ptr<Node> pRoot;
		// sharing meshes here perhaps dangerous?
		std::vector<Mesh*> meshPtrs;
	};
}