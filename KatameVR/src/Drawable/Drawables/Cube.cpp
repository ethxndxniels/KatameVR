#include "Cube.h"

#include "../../Graphics/Graphics.h"
#include "../../Geometry/Geometry.h"

// Bindables
#include "../../Bindables/VertexShader.h"
#include "../../Bindables/PixelShader.h"
#include "../../Bindables/InputLayout.h"
#include "../../Bindables/CBuffer.h"
#include "../../Bindables/Buffer.h"
#include "../../Bindables/Topology.h"

namespace Katame
{
	Cube::Cube( Graphics* gfx )
	{
		VertexShader* m_VS = new VertexShader( gfx, ".\\Shaders\\Bin\\MainVS.cso" );
		Drawable::Submit( m_VS );
		Drawable::Submit( new PixelShader( gfx, ".\\Shaders\\Bin\\MainPS.cso" ) );
		Drawable::Submit( new InputLayout( gfx, Geometry::CreateCubeInputLayout(), *m_VS ) );
		Drawable::Submit( new VertexBuffer( gfx, Geometry::CreateCubeVertices().data(), (unsigned int)Geometry::CreateCubeVertices().size() * sizeof( Geometry::CubeVertex ), sizeof( Geometry::CubeVertex ) ) );
		IndexBuffer* m_IB = new IndexBuffer( gfx, Geometry::CreateCubeIndices().data(), (unsigned int)Geometry::CreateCubeIndices().size() * sizeof( unsigned int ), sizeof( unsigned int ) );
		Drawable::SetIndexBuffer(  m_IB );
		Drawable::Submit( m_IB );
		Drawable::Submit( new Topology( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );
	}

	Cube::~Cube()
	{
	}

	void Cube::Update()
	{
		Pose.position.x += 0.001f;
		//Pose.orientation.x += 0.01f;
		//Pose.orientation.y += 0.01f;
		//Pose.orientation.z += 0.01f;
	}

	DirectX::XMMATRIX Cube::GetModelMatrix()
	{
		return DirectX::XMMatrixTranspose( DirectX::XMMatrixScaling( Scale.x, Scale.y, Scale.z ) * LoadXrPose( Pose ) );
	}

	void Cube::SetData( XrPosef pose, XrVector3f scale )
	{
		Pose = pose;
		Scale = scale;
	}

}