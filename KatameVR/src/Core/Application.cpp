#include "Application.h"
#include "Log.h"

#include "../Geometry/Cube.h"
#include "../Bindables/Sampler.h"

namespace Katame
{
	Application::Application()
	{
		m_XRCore = new XRCore();
		m_XRCore->CreateInstance();
		m_XRCore->InitializeSystem();
		m_XRCore->InitializeDevice();
		m_XRCore->InitializeSession();
		m_XRCore->CreateSwapchains();
	}

	Application::~Application()
	{
	}

	void Application::Launch()
	{
		KM_CORE_INFO( "Launching Application.." );
		while (m_Running) 
		{
			bool exitRenderLoop = false;
			m_XRCore->PollEvents( &exitRenderLoop, &requestRestart );
			if ( exitRenderLoop ) 
			{
				break;
			}

			if ( m_XRCore->IsSessionRunning() )
			{
				m_XRCore->PollActions();
				m_XRCore->RenderFrame();
			}
			else 
			{
				// Throttle loop since xrWaitFrame won't be called.
				std::this_thread::sleep_for( std::chrono::milliseconds( 250 ) );
			}
		}
	}
}