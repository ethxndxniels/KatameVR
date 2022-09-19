#include "NormalCube.h"

#include "../../Graphics/Graphics.h"
#include "../../Geometry/Geometry.h"

// Bindables
#include "../../Bindables/VertexShader.h"
#include "../../Bindables/PixelShader.h"
#include "../../Bindables/InputLayout.h"
#include "../../Bindables/CBuffer.h"
#include "../../Bindables/Buffer.h"
#include "../../Bindables/Topology.h"
#include "../../Bindables/Rasterizer.h"
#include "../../Bindables/Blender.h"

namespace Katame
{
	NormalCube::NormalCube( Graphics* gfx )
	{
		VertexShader* m_VS = new VertexShader( gfx, ".\\Shaders\\Bin\\CubeNormVS.cso" );
		Drawable::Submit( m_VS );
		Drawable::Submit( new PixelShader( gfx, ".\\Shaders\\Bin\\CubeNormPS.cso" ) );
		Drawable::Submit( new InputLayout( gfx, Geometry::CreateNormalCubeInputLayout(), *m_VS ) );
		Drawable::Submit( new VertexBuffer( gfx, Geometry::CreateNormalCubeVertices().data(), (unsigned int)Geometry::CreateNormalCubeVertices().size() * sizeof( Geometry::CubeVertex ), sizeof( Geometry::CubeVertex ) ) );
		IndexBuffer* m_IB = new IndexBuffer( gfx, Geometry::CreateNormalCubeIndices().data(), (unsigned int)Geometry::CreateNormalCubeIndices().size() * sizeof( unsigned short ), sizeof( unsigned short ) );
		Drawable::SetIndexBuffer( m_IB );
		Drawable::Submit( m_IB );
		Drawable::Submit( new Topology( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );
		Drawable::Submit( new Rasterizer( gfx, true ) );
		Drawable::Submit( new Blender( gfx, false ) );
	}

	NormalCube::~NormalCube()
	{
	}

	void NormalCube::Update( float dt )
	{
		Pose.position.x += dt * 0.1f;
		//Pose.orientation.x += 0.01f;
		//Pose.orientation.y += dt * 0.01f;
		//Pose.orientation.z += dt * 0.01f;
	}

	DirectX::XMMATRIX NormalCube::GetModelMatrix()
	{
		return DirectX::XMMatrixTranspose( DirectX::XMMatrixScaling( Scale.x, Scale.y, Scale.z ) * LoadXrPose( Pose ) );
	}

	void NormalCube::SetData( XrPosef pose, XrVector3f scale )
	{
		Pose = pose;
		Scale = scale;
	}

}