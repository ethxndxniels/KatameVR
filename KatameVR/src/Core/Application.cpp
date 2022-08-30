#include "Application.h"
#include "Log.h"

namespace Katame
{
	Application::Application()
	{
		XRCore::Init();
	}

	Application::~Application()
	{
	}

	void Application::Launch()
	{
		KM_CORE_INFO( "Launching Application.." );
		while (m_Running) 
		{
			float dt = timer.Mark();
			Update( dt );
		}

	}

	void Application::Draw( XrCompositionLayerProjectionView& view ) 
	{
	}

	void Application::Update( float dt )
	{
		KM_CORE_TRACE( "Time Elapsed: {}", dt );
	}

	void Application::Update_Predicted()
	{
	}

}