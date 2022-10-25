#include "Node.h"
#include "Mesh.h"

namespace dx = DirectX;

namespace Katame
{
	Node::Node( int id, const std::string& name, std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX& transform_in ) noexcept
		:
		id( id ),
		meshPtrs( std::move( meshPtrs ) ),
		name( name )
	{
		dx::XMStoreFloat4x4( &transform, transform_in );
		dx::XMStoreFloat4x4( &appliedTransform, dx::XMMatrixIdentity() );
	}

	void Node::Submit( DirectX::FXMMATRIX accumulatedTransform ) const noexcept
	{
		const auto built =
			dx::XMLoadFloat4x4( &appliedTransform ) *
			dx::XMLoadFloat4x4( &transform ) *
			accumulatedTransform;
		for (const auto pm : meshPtrs)
		{
			//pm->Submit( built );
		}
		for (const auto& pc : childPtrs)
		{
			//pc->Submit( built );
		}
	}

	void Node::AddChild( std::unique_ptr<Node> pChild ) noexcept
	{
		assert( pChild );
		childPtrs.push_back( std::move( pChild ) );
	}

	void Node::SetAppliedTransform( DirectX::FXMMATRIX transform ) noexcept
	{
		dx::XMStoreFloat4x4( &appliedTransform, transform );
	}

	const DirectX::XMFLOAT4X4& Node::GetAppliedTransform() const noexcept
	{
		return appliedTransform;
	}

	int Node::GetId() const noexcept
	{
		return id;
	}
}