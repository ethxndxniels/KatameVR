#include "Application.h"

#include "Log.h"

namespace Katame
{
	Application::Application()
	{
		m_Graphics = new Graphics();
		m_XRCore = new XRCore( m_Graphics );
		m_XRCore->CreateInstance();
		m_XRCore->InitializeSystem();
		m_XRCore->InitializeDevice();
		m_XRCore->InitializeSession();
		m_XRCore->CreateSwapchains();
		m_Renderer = new Renderer( m_Graphics );
		m_Graphics->InitializeRenderer( m_Renderer );

        m_Cube = new Cube( m_Graphics );
		m_Cube->SetData( { { 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } }, { 0.25f, 0.25f, 0.25f } );
		m_Cube2 = new Cube( m_Graphics );
		m_Cube2->SetData( { { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } }, { 0.25f, 0.25f, 0.25f } );
	}

	Application::~Application()
	{
	}

	void Application::Launch()
	{
		//KM_CORE_INFO( "Launching Application.." );
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
				Update( 1.0f );
				Submit();
				m_XRCore->RenderFrame();
			}
			else 
			{
				// Throttle loop since xrWaitFrame won't be called.
				std::this_thread::sleep_for( std::chrono::milliseconds( 250 ) );
			}
		}
	}

	void Application::Update( float dt )
	{
		m_Cube->Update();
		m_Cube2->Update();
	}

	void Application::Submit()
	{
		m_Renderer->Submit( *m_Cube );
		m_Renderer->Submit( *m_Cube2 );
	}
}