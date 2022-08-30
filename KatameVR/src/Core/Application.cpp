#include "Application.h"
#include "Log.h"

namespace Katame
{
	Application::Application()
	{

	}

	Application::~Application()
	{
	}

	void Application::Launch()
	{
		KM_CORE_INFO( "Launching Application.." );
		while (m_Running) 
		{

		}

	}

	void Application::Draw( XrCompositionLayerProjectionView& view ) 
	{
	
	}

	void Application::Update( float dt )
	{
	}

	void Application::Update_Predicted()
	{
	}

}