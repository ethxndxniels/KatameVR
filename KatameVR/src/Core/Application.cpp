#include "Application.h"
#include "Log.h"

namespace Katame
{
	Application::Application()
	{
		XRCore::Init();
		XRRender::Init();

		// Action Set
		XRInput::CreateActionSet( "main", "main", 0 );
		m_ActionState_PoseLeft.type = XR_TYPE_ACTION_STATE_POSE;
		m_Action_PoseLeft = XRInput::CreateAction( XRInput::ActionSets()[0], "pose_left", "Pose (Left)", XR_ACTION_TYPE_POSE_INPUT, 0, NULL );
		m_ActionState_PoseRight.type = XR_TYPE_ACTION_STATE_POSE;
		m_Action_PoseRight = XRInput::CreateAction( XRInput::ActionSets()[0], "pose_right", "Pose (Right)", XR_ACTION_TYPE_POSE_INPUT, 0, NULL );
		m_ActionState_SwitchScene.type = XR_TYPE_ACTION_STATE_BOOLEAN;
		m_Action_SwitchScene = XRInput::CreateAction( XRInput::ActionSets()[0], "switch_scene", "Switch Scenes", XR_ACTION_TYPE_BOOLEAN_INPUT, 0, NULL );
		m_Action_Haptic = XRInput::CreateAction( XRInput::ActionSets()[0], "haptic", "Haptic Feedback", XR_ACTION_TYPE_VIBRATION_OUTPUT, 0, NULL );
		XRInput::CreateActionBinding( m_Action_PoseLeft, Hand_Left, Pose_Grip, &m_SuggestedBindings );
		XRInput::CreateActionBinding( m_Action_PoseRight, Hand_Right, Pose_Grip, &m_SuggestedBindings );
		XRInput::CreateActionBinding( m_Action_SwitchScene, Hand_Left, Button_Trigger_Click, &m_SuggestedBindings );
		XRInput::CreateActionBinding( m_Action_SwitchScene, Hand_Right, Button_Trigger_Click, &m_SuggestedBindings );
		XRInput::CreateActionBinding( m_Action_Haptic, Hand_Left, Output_Haptic, &m_SuggestedBindings );
		XRInput::CreateActionBinding( m_Action_Haptic, Hand_Right, Output_Haptic, &m_SuggestedBindings );
		XRInput::SuggestActionBindings( &m_SuggestedBindings, "/interaction_profiles/oculus/touch_controller" );
		XRInput::ActivateActionSet( XRInput::ActionSets()[0] );

		// Hand Tracking?
		// bDrawHandJoints = pXRHandTracking->IsActive();

		// Swapchain capacity
		u_SwapChainCount = XRGraphics::GetSwapchainImageCount( Katame::EYE_LEFT );
		if (u_SwapChainCount < 1)
			KM_CORE_ERROR( "Not enough swapchain capacity ({}) to do any rendering work", u_SwapChainCount );

		// TODO:
		// Event callback
		//Katame::XRCallback xrCallback = { XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED };	// XR_TYPE_EVENT_DATA_BUFFER = Register for all events
		//Katame::XRCallback* pXRCallback = &xrCallback;
		//pXRCallback->callback = &Application::OnEvent;
		//XREventHandler::RegisterCallback( pXRCallback );

		// TODO: Sort Vis Mask Out
		//XRRender::GetVisibilityMask( Katame::EYE_LEFT, XRRender::EMaskType::MASK_HIDDEN, m_MaskVertices_L, m_MaskIndices_L );
		//XRRender::GetVisibilityMask( Katame::EYE_RIGHT, XRRender::EMaskType::MASK_HIDDEN, m_MaskVertices_R, m_MaskIndices_R );

		// Graphics Resources
		std::vector<VertexPosColor> cubeVertices = CreateCubeVertices();
		std::vector<unsigned int> cubeIndices = CreateCubeIndices();
		CD3D11_BUFFER_DESC cBufDesc( sizeof( TransformBuffer ), D3D11_BIND_CONSTANT_BUFFER );

		m_VB = new VertexBuffer( cubeVertices.data(), cubeVertices.size(), sizeof( VertexPosColor ) );
		m_IB = new IndexBuffer( cubeIndices.data(), cubeIndices.size(), sizeof( unsigned int ) );
		m_VS = new VertexShader( "./Shaders/Bin/VertexShader.cso" );
		m_PS = new PixelShader( "./Shaders/Bin/PixelShader.cso" );
		m_IL = new InputLayout( CreateCubeInputLayout(), *m_VS );
		m_Top = new Topology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		m_VCBuf = new VCBuffer( cBufDesc, sizeof( cBufDesc ), 0u );
	}

	Application::~Application()
	{
	}

	int nSwapchainIndex = 0;
	void Application::Launch()
	{
		KM_CORE_INFO( "Launching Application.." );
		while ( m_Running ) 
		{         
			XRCore::PollEvents( m_Running, std::bind( &Application::OnEvent, this, std::placeholders::_1  ) );
			float dt = timer.Mark();
			Update( dt );

			if ( XRCore::IsRunning() )
			{
				// Poll Actions
				XRRender::ProcessXRFrame();
				Draw();
				XRInput::SyncActiveActionSetsData();
				ProcessInputStates();
				uint64_t nPredictedTime = XRRender::GetPredictedDisplayTime() + XRRender::GetPredictedDisplayPeriod();

				XRInput::GetActionPose( m_Action_PoseLeft, nPredictedTime, &m_Location_Left );
				XRInput::GetActionPose( m_Action_PoseRight, nPredictedTime, &m_Location_Right );
				
				

				// Blit (copy) texture to XR Mirror
				//BlitToWindow();
			}
		}

	}

	void Application::Draw()
	{
		m_VB->Bind();
		m_IB->Bind();
		m_VS->Bind();
		m_PS->Bind();
		m_IL->Bind();
		m_Top->Bind();

		TransformBuffer tBuf;
		XMStoreFloat4x4( &tBuf.ViewProj, DirectX::XMMatrixTranspose( XRRender::GetView( 0 ) * XRRender::GetProjection( 0 ) ) );
		DirectX::XMMATRIX model = XMMatrixAffineTransformation(
			DirectX::g_XMOne * 0.05f, DirectX::g_XMZero,
			DirectX::XMLoadFloat4( &cubeOrientation ),
			DirectX::XMLoadFloat3( &cubePos ) );
		XMStoreFloat4x4( &tBuf.World, DirectX::XMMatrixTranspose( model ) );
		m_VCBuf->Update( &tBuf );
		m_VCBuf->Bind();

		XRGraphics::GetContext()->DrawIndexed( (UINT)CreateCubeIndices().size(), 0, 0 );

		XMStoreFloat4x4( &tBuf.ViewProj, DirectX::XMMatrixTranspose( XRRender::GetView( 1 ) * XRRender::GetProjection( 1 ) ) );
		m_VCBuf->Update( &tBuf );
		m_VCBuf->Bind();

		XRGraphics::GetContext()->DrawIndexed( (UINT)CreateCubeIndices().size(), 0, 0 );
	}

	void Application::OnEvent( XrEventDataBuffer& e )
	{
		KM_INFO( "Processing Event: {}", e.type );
	}

	void Application::ProcessInputStates()
	{
		if (XRInput::GetActionStateBoolean( m_Action_SwitchScene, &m_ActionState_SwitchScene ) == XR_SUCCESS
			&& m_ActionState_SwitchScene.changedSinceLastSync && m_ActionState_SwitchScene.currentState)
		{
			// Switch active scene
			m_CurrentScene = m_CurrentScene == SCENE_HAND_TRACKING ? SCENE_SEA_OF_CUBES : SCENE_HAND_TRACKING;

			// Apply haptic
			XRInput::GenerateHaptic( m_Action_Haptic, XR_MIN_HAPTIC_DURATION, 0.5f, XR_FREQUENCY_UNSPECIFIED );
			KM_INFO( "Input Detected: Action Switch Scene ({}) last changed on ({}) nanoseconds",
				(bool)m_ActionState_SwitchScene.currentState, (uint64_t)m_ActionState_SwitchScene.lastChangeTime );
		}
	}

	void Application::Update( float dt )
	{
	}

	void Application::Update_Predicted()
	{
	}

}