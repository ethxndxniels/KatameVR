#include "Application.h"

#include "Log.h"

namespace Katame
{
	Application::Application()
	{
		// D3D11 and OpenXR
		m_Graphics = new Graphics();
		m_XRCore = new XRCore( m_Graphics );
		m_XRCore->CreateInstance();
		m_XRCore->InitializeSystem();
		m_XRCore->InitializeDevice();
		m_XRCore->InitializeSession();
		m_XRCore->CreateSwapchains();

		// Renderer
		m_Renderer = new Renderer( m_Graphics );
		m_Graphics->InitializeRenderer( m_Renderer );

		// Win32
		m_Window = new Win32Window( 10, 10, "Desktop" );

		// Entities
		m_Sponza = new Model( m_Graphics, "Models\\sponza\\sponza.obj", 0.125f );

		// Player
		m_Hands = new Hands( m_Graphics, m_XRCore );

		// Lights
		m_PointLight = new PointLight(m_Graphics);
		m_DirLight = new DirLight( m_Graphics );
	}

	Application::~Application()
	{
		delete m_XRCore;
		delete m_Graphics;
		delete m_Renderer;
		delete m_Window;
		delete m_Sponza;
		delete m_Hands;
		delete m_PointLight;
		delete m_DirLight;
	}

	void Application::Launch()
	{
		KM_CORE_INFO( "Launching Application.." );
		while (m_Running) 
		{
			bool exitRenderLoop = false;
			m_XRCore->PollEvents( &exitRenderLoop, &requestRestart );
			m_Window->ProcessMessages();
			
			if ( exitRenderLoop ) 
				break;

			if ( m_XRCore->IsSessionRunning() )
			{
				m_XRCore->PollActions();
				const auto dt = m_Timer.Mark() * speed_factor;
				Update( dt );
				Submit();
				m_XRCore->RenderFrame();
				m_Renderer->Clear();
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
		m_Hands->Update( dt );
		m_DirLight->Update( dt );
		m_PointLight->Update( dt );
	}

	void Application::Submit()	
	{
		// Lights
		m_DirLight->Bind();
		m_Renderer->Submit( *m_PointLight );


		// Entities
		m_Renderer->Submit( *m_Sponza );
		
		// Hands
		m_Renderer->Submit( *m_Hands->GetLeftHand() );
		m_Renderer->Submit( *m_Hands->GetRightHand() );
	}
}