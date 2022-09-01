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

		// TODO: Setup Graphics Resources
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
				// ProcessInputStates
				// Blit (copy) texture to XR Mirror
				//BlitToWindow();
			}
		}

	}

	void Application::Draw()
	{
	}

	void Application::OnEvent( XrEventDataBuffer& e )
	{
		KM_INFO( "OI" );
	}

	void Application::Update( float dt )
	{
		//KM_TRACE( "Time Elapsed: {}", dt );
	}

	void Application::Update_Predicted()
	{
	}

}