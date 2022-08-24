#pragma once
#include "../Drawable/Drawable.h"
#include "../Bindables/Bindable.h"
#include "../Graphics/Vertex.h"
#include <optional>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../Bindables/ConstantBuffers.h"
#include <type_traits>
#include <filesystem>
#include "../Bindables/Texture.h"
#include "../Bindables/Sampler.h"
#include "../Bindables/DynamicConstant.h"

namespace Katame
{

	class Mesh : public Drawable
	{
	public:
		Mesh(Graphics* gfx, bool isAlpha, std::vector<std::shared_ptr<Bind::Bindable>> bindPtrs );
		bool HasAlpha();
		void Draw(Graphics* gfx, DirectX::FXMMATRIX accumulatedTransform, bool isDepthPass ) const noexcept;
		DirectX::XMMATRIX GetTransformXM() const noexcept override;
	private:
		bool isAlpha = false;
		mutable DirectX::XMFLOAT4X4 transform;
	};

	class Node
	{
		friend class Model;
	public:
		Node( int id, const std::string& name, std::vector<Mesh*> meshPtrs, std::vector<Mesh*> meshPtrsAlpha, const DirectX::XMMATRIX& transform ) noexcept;
		void Draw(Graphics* gfx, DirectX::FXMMATRIX accumulatedTransform, bool isDepthPass ) const noexcept;
		void SetAppliedTransform( DirectX::FXMMATRIX transform ) noexcept;
		const DirectX::XMFLOAT4X4& GetAppliedTransform() const noexcept;
		int GetId() const noexcept;
		void ShowTree( Node*& pSelectedNode ) const noexcept;
		const Dcb::Buffer* GetMaterialConstants() const noexcept;
		void SetMaterialConstants( const Dcb::Buffer& ) noexcept;
	private:
		void AddChild( std::unique_ptr<Node> pChild ) noexcept;
	private:
		std::string name;
		int id;
		std::vector<std::unique_ptr<Node>> childPtrs;
		std::vector<Mesh*> meshPtrs;
		std::vector<Mesh*> meshPtrsAlpha;
		DirectX::XMFLOAT4X4 transform;
		DirectX::XMFLOAT4X4 appliedTransform;
	};

	class Model
	{
	public:
		Model(Graphics* gfx, const std::string& pathString, bool isForward, float scale = 1.0f );
		void Draw(Graphics* gfx, bool isDepthPass ) const noexcept;
		void ShowWindow(Graphics* gfx, const char* windowName = nullptr ) noexcept;
		void SetRootTransform( DirectX::FXMMATRIX tf ) noexcept;
		~Model() noexcept;
	private:
		static std::unique_ptr<Mesh> ParseMesh(Graphics* gfx, const aiMesh& mesh, const aiMaterial* const* pMaterials, const std::filesystem::path& path, bool isForward, float scale );
		std::unique_ptr<Node> ParseNode( int& nextId, const aiNode& node ) noexcept;
	private:
		std::unique_ptr<Node> pRoot;
		std::vector<std::unique_ptr<Mesh>> meshPtrs;
		std::unique_ptr<class ModelWindow> pWindow;
	};

}