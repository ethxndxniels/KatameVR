#pragma once

#include "../Graphics/Graphics.h"
#include <string>
#include <vector>
#include <memory>

namespace Katame
{
	class Model;
	class Mesh;
	class TechniqueProbe;
	class ModelProbe;

	class Node
	{
		friend class Model;
	public:
		Node( int id, const std::string& name, std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX& transform ) noexcept;
		void Render( Graphics* gfx, DirectX::FXMMATRIX accumulatedTranform );
		void SetAppliedTransform( DirectX::FXMMATRIX transform ) noexcept;
		const DirectX::XMFLOAT4X4& GetAppliedTransform() const noexcept;
		int GetId() const noexcept;
		//void ShowTree( Node*& pSelectedNode ) const noexcept;
		bool HasChildren() const noexcept
		{
			return childPtrs.size() > 0;
		}
		const std::string& GetName() const
		{
			return name;
		}
	private:
		void AddChild( std::unique_ptr<Node> pChild ) noexcept;
	private:
		std::string name;
		int id;
		std::vector<std::unique_ptr<Node>> childPtrs;
		std::vector<Mesh*> meshPtrs;
		DirectX::XMFLOAT4X4 transform;
		DirectX::XMFLOAT4X4 appliedTransform;
	};
}
