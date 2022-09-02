#pragma once

// STD
#include <string>
#include <thread>
#include <vector>
#include <algorithm>

// OpenXR Abstraction
#include "../XR/XRCore.h"
#include "../XR/XRInput.h"
#include "../XR/XRHandTracking.h"
#include "../XR/XRRender.h"
#include "../XR/XRGraphics.h"
#include "../XR/XROculusTouchDefines.h"
#include "../XR/XRMirror.h"

// Bindables
#include "../Bindables/Buffer.h"
#include "../Bindables/VertexShader.h"
#include "../Bindables/PixelShader.h"
#include "../Bindables/InputLayout.h"
#include "../Bindables/Topology.h"
#include "../Bindables/CBuffer.h"

// Utilities
#include "../Utilities/FrameTimer.h"

// Geometry
#include "../Geometry/Cube.h"

namespace Katame 
{
	enum Scenes
	{
		SCENE_SEA_OF_CUBES = 0,
		SCENE_HAND_TRACKING = 1
	};

	struct TransformBuffer
	{
		DirectX::XMFLOAT4X4 World;
		DirectX::XMFLOAT4X4 ViewProj;
	};

	class Application
	{
	public:
		Application();
		~Application();
		void Launch();
	public:
		void Draw();
		void OnEvent( XrEventDataBuffer& e );
		void ProcessInputStates();
		void Update( float dt );
		void Update_Predicted();
	private:
		bool m_Running = true, m_Minimized = false;
		bool xr_running = false;
		FrameTimer timer;
		float speed_factor = 1.0f;
		uint32_t u_SwapChainCount = 0;

		XRMirror* m_Mirror;

		// Actions
		XrAction m_Action_SwitchScene, m_Action_Haptic;
		XrAction m_Action_PoseLeft, m_Action_PoseRight;
		XrActionStateBoolean m_ActionState_SwitchScene;
		XrActionStatePose m_ActionState_PoseLeft;
		XrActionStatePose m_ActionState_PoseRight;
		XrSpaceLocation m_Location_Left{ XR_TYPE_SPACE_LOCATION };
		XrSpaceLocation m_Location_Right{ XR_TYPE_SPACE_LOCATION };
		std::vector<XrActionSuggestedBinding> m_SuggestedBindings;
		bool bDrawHandJoints = false;

		// Mask Data
		std::vector<float> m_MaskVertices_L;
		std::vector<float> m_MaskVertices_R;
		std::vector<uint32_t> m_MaskIndices_L;
		std::vector<uint32_t> m_MaskIndices_R;

		Scenes m_CurrentScene = SCENE_SEA_OF_CUBES;

		DirectX::XMFLOAT3 cubePos = { 1.0f, 1.0f, 1.0f };
		DirectX::XMFLOAT4 cubeOrientation = { 0.0f, 0.0f, 0.0f, 0.0f };

		// Bindables
		IndexBuffer* m_IB;
		VertexBuffer* m_VB;
		VertexShader* m_VS;
		PixelShader* m_PS;
		InputLayout* m_IL;
		Topology* m_Top;
		VCBuffer* m_VCBuf;
	};
}
