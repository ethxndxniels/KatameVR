#include "Application.h"

#include "Log.h"

namespace Katame
{
	Application::Application()
		:
		m_Graphics(),
		m_XRCore( m_Graphics ),
		m_Renderer( m_Graphics ),
		m_Window( 10, 10, "Desktop" ),
		m_Sponza( m_Graphics, "Models\\sponza\\sponza.obj", 0.125f ),
		m_Hands( m_Graphics, m_XRCore ),
		m_PointLight( m_Graphics ),
		m_DirLight( m_Graphics )
	{
		// Renderer
		m_Graphics.InitializeRenderer( m_Renderer );
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
			m_XRCore.PollEvents( &exitRenderLoop, &requestRestart );
			m_Window.ProcessMessages();
			
			if ( exitRenderLoop ) 
				break;

			if ( m_XRCore.IsSessionRunning() )
			{
				m_XRCore.PollActions();
				const auto dt = m_Timer.Mark() * speed_factor;
				Update( dt );
				Submit();
				m_XRCore.RenderFrame();
				m_Renderer.Clear();
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
		m_Hands.Update( dt );
		m_DirLight.Update( dt );
		m_PointLight.Update( dt );
		
	}

	void Application::Submit()	
	{
		// Lights
		m_DirLight.Bind();
		m_Renderer.Submit( m_PointLight );


		// Entities
		m_Renderer.Submit( m_Sponza );
		
		// Hands
		m_Renderer.Submit( m_Hands.GetLeftHand() );
		m_Renderer.Submit( m_Hands.GetRightHand() );
	}
}