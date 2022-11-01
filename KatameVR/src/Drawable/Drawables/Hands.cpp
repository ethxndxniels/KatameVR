#include "Hands.h"

#include "../../Graphics/Graphics.h"
#include "../../Geometry/Geometry.h"

#include "../../XR/XRCore.h"

// Bindables
#include "../../Bindables/VertexShader.h"
#include "../../Bindables/PixelShader.h"
#include "../../Bindables/InputLayout.h"
#include "../../Bindables/CBuffer.h"
#include "../../Bindables/Buffer.h"
#include "../../Bindables/Topology.h"
#include "../../Bindables/Rasterizer.h"
#include "../../Bindables/Blender.h"

#include "../../Renderer/Model.h"

#include "./NormalCube.h"

namespace Katame
{
	Hands::Hands( Graphics* gfx, XRCore* xrCore )
		: gfx( gfx ), xrCore( xrCore )
	{
		//m_LeftHand = new NormalCube( gfx );
		//m_RightHand = new NormalCube( gfx );
		m_LeftHand = new Model( gfx, "Models\\hand-for-vr\\source\\Player hand.fbx" );
		m_RightHand = new Model( gfx, "Models\\hand-for-vr\\source\\Player hand.fbx" );
	}

	Hands::~Hands()
	{
		delete m_LeftHand;
		delete m_RightHand;
	}

	void Hands::Update( float dt )
	{
		m_LeftHand->SetData( xrCore->GetLeftHand().first, xrCore->GetLeftHand().second );
		m_RightHand->SetData( xrCore->GetRightHand().first, xrCore->GetRightHand().second );
	}
	Model* Hands::GetLeftHand()
	{
		return m_LeftHand;
	}
	Model* Hands::GetRightHand()
	{
		return m_RightHand;
	}
}