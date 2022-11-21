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
		m_LeftHand = new Model( gfx, "Models\\hand\\hand.fbx" );
		m_RightHand = new Model( gfx, "Models\\hand\\hand.fbx" );
	}

	Hands::~Hands()
	{
		delete m_LeftHand;
		delete m_RightHand;
	}

	void Hands::Update( float dt )
	{
		// hard coded scale takes away user inputted scale from xrCore->GetRightHand().second
		XrVector3f scale = { 0.00025f, 0.00025f, 0.00025f };
		m_LeftHand->SetData( xrCore->GetLeftHand().first, scale);
		m_RightHand->SetData( xrCore->GetRightHand().first, scale);
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