///////////////////////////////////////////
// Main                                  //
///////////////////////////////////////////

// Katame Stuff
#include "Core/Application.h"
//

int __stdcall wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int ) {

	
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
				//this_thread::sleep_for( chrono::milliseconds( 250 ) );
			}
		}
	}

	app.openxr_shutdown();
	app.d3d_shutdown();
	return 0;
}
