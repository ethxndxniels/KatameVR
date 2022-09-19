#include "ColorCube.h"

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
	ColorCube::ColorCube( Graphics* gfx )
	{
		VertexShader* m_VS = new VertexShader( gfx, ".\\Shaders\\Bin\\MainVS.cso" );
		Drawable::Submit( m_VS );
		Drawable::Submit( new PixelShader( gfx, ".\\Shaders\\Bin\\MainPS.cso" ) );
		Drawable::Submit( new InputLayout( gfx, Geometry::CreateColorCubeInputLayout(), *m_VS ) );
		Drawable::Submit( new VertexBuffer( gfx, Geometry::CreateColorCubeVertices().data(), (unsigned int)Geometry::CreateColorCubeVertices().size() * sizeof( Geometry::CubeVertex ), sizeof( Geometry::CubeVertex ) ) );
		IndexBuffer* m_IB = new IndexBuffer( gfx, Geometry::CreateColorCubeIndices().data(), (unsigned int)Geometry::CreateColorCubeIndices().size() * sizeof( unsigned short ), sizeof( unsigned short ) );
		Drawable::SetIndexBuffer(  m_IB );
		Drawable::Submit( m_IB );
		Drawable::Submit( new Topology( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );
		Drawable::Submit( new Rasterizer( gfx, true ) );
		Drawable::Submit( new Blender( gfx, false ) );
	}

	ColorCube::~ColorCube()
	{
	}

	void ColorCube::Update( float dt )
	{
		Pose.position.x += dt * 0.1f;
		//Pose.orientation.x += 0.01f;
		//Pose.orientation.y += dt * 0.01f;
		//Pose.orientation.z += dt * 0.01f;
	}

	DirectX::XMMATRIX ColorCube::GetModelMatrix()
	{
		return DirectX::XMMatrixTranspose( DirectX::XMMatrixScaling( Scale.x, Scale.y, Scale.z ) * LoadXrPose( Pose ) );
	}

	void ColorCube::SetData( XrPosef pose, XrVector3f scale )
	{
		Pose = pose;
		Scale = scale;
	}

}