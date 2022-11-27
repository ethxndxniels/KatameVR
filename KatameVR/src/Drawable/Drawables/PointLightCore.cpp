#include "PointLightCore.h"

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
	static std::vector<D3D11_INPUT_ELEMENT_DESC> CreateFlatColorInputLayout()
	{
		return
		{
			{"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Color", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}

	struct VertexColor
	{
		XrVector3f Position;
		XrVector3f Color;
	};

	static std::vector<VertexColor> CreateFlatCubeVertices( float r, float g, float b )
	{
#define CUBE_SIDE(V1, V2, V3, V4, V5, V6, COLOR) {V1, COLOR}, {V2, COLOR}, {V3, COLOR}, {V4, COLOR}, {V5, COLOR}, {V6, COLOR},
		constexpr XrVector3f LBB{ -0.5f, -0.5f, -0.5f };
		constexpr XrVector3f LBF{ -0.5f, -0.5f, 0.5f };
		constexpr XrVector3f LTB{ -0.5f, 0.5f, -0.5f };
		constexpr XrVector3f LTF{ -0.5f, 0.5f, 0.5f };
		constexpr XrVector3f RBB{ 0.5f, -0.5f, -0.5f };
		constexpr XrVector3f RBF{ 0.5f, -0.5f, 0.5f };
		constexpr XrVector3f RTB{ 0.5f, 0.5f, -0.5f };
		constexpr XrVector3f RTF{ 0.5f, 0.5f, 0.5f };
		XrVector3f color{ r, g, b };

		return std::vector<VertexColor>
		{
			CUBE_SIDE(LTB, LBF, LBB, LTB, LTF, LBF, color)    // -X
			CUBE_SIDE(RTB, RBB, RBF, RTB, RBF, RTF, color)        // +X
			CUBE_SIDE(LBB, LBF, RBF, LBB, RBF, RBB, color)  // -Y
			CUBE_SIDE(LTB, RTB, RTF, LTB, RTF, LTF, color)      // +Y
			CUBE_SIDE(LBB, RBB, RTB, LBB, RTB, LTB, color)   // -Z
			CUBE_SIDE(LBF, LTF, RTF, LBF, RTF, RBF, color)       // +Z
		};
	}

	PointLightCore::PointLightCore(Graphics* gfx)
	{
		VertexShader* m_VS = new VertexShader(gfx, ".\\Shaders\\Bin\\FlatColorVS.cso");
		Drawable::AddBind(m_VS);
		Drawable::AddBind(new PixelShader(gfx, ".\\Shaders\\Bin\\FlatColorPS.cso"));
		Drawable::AddBind(new InputLayout(gfx, CreateFlatColorInputLayout(), *m_VS));
		Drawable::AddBind(new VertexBuffer(gfx, CreateFlatCubeVertices( 1.0f, 1.0f, 1.0f ).data(), (unsigned int)CreateFlatCubeVertices(1.0f, 1.0f, 1.0f).size() * sizeof(VertexColor), sizeof(VertexColor)));
		IndexBuffer* m_IB = new IndexBuffer(gfx, Geometry::CreateNormalCubeIndices().data(), (unsigned int)Geometry::CreateNormalCubeIndices().size() * sizeof(unsigned int), sizeof(unsigned int));
		Drawable::SetIndexBuffer(m_IB);
		Drawable::AddBind(m_IB);
		Drawable::AddBind(new Topology(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		Drawable::AddBind(new Rasterizer(gfx, true));
		Drawable::AddBind(new Blender(gfx, false));
	}

	PointLightCore::~PointLightCore()
	{
	}

	void PointLightCore::Update(float dt)
	{
		Pose.position.x += dt * 0.1f;
		//Pose.orientation.x += 0.01f;
		//Pose.orientation.y += dt * 0.01f;
		//Pose.orientation.z += dt * 0.01f;
	}

	DirectX::XMMATRIX PointLightCore::GetModelMatrix()
	{
		return DirectX::XMMatrixTranspose(DirectX::XMMatrixScaling(Scale.x, Scale.y, Scale.z) * LoadXrPose(Pose));
	}

	void PointLightCore::SetData(XrPosef pose, XrVector3f scale)
	{
		Pose = pose;
		Scale = scale;
	}

}