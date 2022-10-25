#include "Mesh.h"
#include "../Core/Log.h"

#include "../Graphics/Graphics.h"
#include <filesystem>

namespace Katame 
{
	namespace dx = DirectX;

	// Mesh
	Mesh::Mesh( Graphics* gfx, const Material& mat, const aiMesh& mesh, float scale )
	{
	}

	Mesh::~Mesh()
	{
	}

	DirectX::XMMATRIX Mesh::GetModelMatrix() 
	{
		return DirectX::XMLoadFloat4x4( &transform );
	}
}