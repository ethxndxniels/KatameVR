#include "src/Core/Application.h"
#include "src/Core/Log.h"

int main( int argc, char** argv )
{
	spdlog::info( "Welcome to spdlog!" );
	Katame::Log::Init();
	KM_CORE_WARN( "Initialized Log!" );
	int a = 5;
	KM_INFO( "Hello! Var={0}", a );

	Katame::Application app;

	bool quit = false;
	while (!quit) {
		app.openxr_poll_events( quit );

		if (app.get_xr_running()) {
			app.openxr_poll_actions();
			app.Update();
			app.openxr_render_frame();
			if (app.get_session_state() != XR_SESSION_STATE_VISIBLE &&
				app.get_session_state() != XR_SESSION_STATE_FOCUSED) {
				std::this_thread::sleep_for( std::chrono::milliseconds( 250 ) );
			}
		}
	}

	app.openxr_shutdown();
	app.d3d_shutdown();
	return 0;
}
