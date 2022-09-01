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
		uint32_t u_SwapChainCount = XRGraphics::GetSwapchainImageCount( Katame::EYE_LEFT );
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


	/*
			openxrManager->openxr_poll_events( m_Running, xr_running );
			const auto dt = timer.Mark() * speed_factor;
			if (xr_running) {
				openxrManager->openxr_poll_actions();
				Update( dt );
				openxrManager->openxr_render_frame( this );
				if (openxrManager->get_session_state() != XR_SESSION_STATE_VISIBLE &&
					openxrManager->get_session_state() != XR_SESSION_STATE_FOCUSED) {
					std::this_thread::sleep_for( std::chrono::milliseconds( 250 ) );
				}
			}
	*/
	void Application::Launch()
	{
		KM_CORE_INFO( "Launching Application.." );
		while ( m_Running ) 
		{         
			XRCore::PollEvents( m_Running );
			float dt = timer.Mark();
			Update( dt );

			if ( XRCore::IsRunning() )
			{
				// Poll Actions
				XRRender::ProcessXRFrame();

			}
		}

	}

	void Application::Draw()
	{
	}

	void Application::OnEvent( XrEventDataBuffer e )
	{
	}

	void Application::Update( float dt )
	{
		//KM_CORE_TRACE( "Time Elapsed: {}", dt );
	}

	void Application::Update_Predicted()
	{
	}

}