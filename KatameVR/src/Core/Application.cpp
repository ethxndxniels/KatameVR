#include "Application.h"

#include "Log.h"

namespace Katame
{
	Application::Application()
	{
		// Core
		m_Graphics = new Graphics();
		m_XRCore = new XRCore( m_Graphics );
		m_XRCore->CreateInstance();
		m_XRCore->InitializeSystem();
		m_XRCore->InitializeDevice();
		m_XRCore->InitializeSession();
		m_XRCore->CreateSwapchains();
		m_Renderer = new Renderer( m_Graphics );
		m_Graphics->InitializeRenderer( m_Renderer );
		//m_Window = new Win32Window( 1280, 720, "Desktop" );

		// Entities
		m_Cube = new ColorCube( m_Graphics );
		m_Cube->SetData( { { -1.0f, -1.0f, -1.0f, -1.0f }, { -1.0f, -1.0f, -1.0f } }, { 0.025f, 0.025f, 0.025f } );
		m_Cube2 = new ColorCube( m_Graphics );
		m_Cube2->SetData( { { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } }, { 0.025f, 0.025f, 0.025f } );
		m_NCube = new NormalCube( m_Graphics );
		m_NCube->SetData( { { -1.0f, -1.0f, -1.0f, -1.0f }, { -1.0f, -1.0f, -1.0f } }, { 0.025f, 0.025f, 0.025f } );
		m_NCube2 = new NormalCube( m_Graphics );
		m_NCube2->SetData( { { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } }, { 0.025f, 0.025f, 0.025f } );
		m_Mesh = new Mesh( "Models\\cerberus\\cerberus.fbx", m_Graphics );
		m_Mesh->SetData( { { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -5.0f } }, { 0.025f, 0.025f, 0.025f } );
		//m_Mesh2 = new Mesh( "Models\\Sponza\\sponza.obj", m_Graphics );
		//m_Mesh2->SetData( { { 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } }, { 0.025f, 0.025f, 0.025f } );

		// Player
		m_Hands = new Hands( m_Graphics, m_XRCore );

		// Lights
		m_DirLight = new DirLight( m_Graphics );
	}

	Application::~Application()
	{
		delete m_XRCore;
		delete m_Graphics;
		delete m_Renderer;
		delete m_Window;
		delete m_Cube;
		delete m_Cube2;
		delete m_Mesh;
		delete m_Mesh2;
		delete m_DirLight;
		delete m_NCube;
		delete m_NCube2;
		delete m_Hands;
	}

	void Application::Launch()
	{
		KM_CORE_INFO( "Launching Application.." );
		while (m_Running) 
		{
			bool exitRenderLoop = false;
			m_XRCore->PollEvents( &exitRenderLoop, &requestRestart );
			//m_Window->ProcessMessages();
			
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
		//m_Cube2->Update( -dt );
		m_NCube->Update( dt );
		m_NCube2->Update( -dt );
		m_Mesh->Update( dt );
		//m_Mesh2->Update( dt );
		m_Hands->Update( dt );
		m_DirLight->Update( dt );
	}

	void Application::Submit()	
	{
		// Lights
		m_DirLight->Bind();

		// Entities
		m_Renderer->Submit( *m_Cube );
		m_Renderer->Submit( *m_Cube2 );
		m_Renderer->Submit( *m_NCube );
		m_Renderer->Submit( *m_NCube2 );
		m_Renderer->Submit( *m_Mesh );
		//m_Renderer->Submit( *m_Mesh2 );
		m_Renderer->Submit( *m_Hands->GetLeftHand() );
		m_Renderer->Submit( *m_Hands->GetRightHand() );
	}
}