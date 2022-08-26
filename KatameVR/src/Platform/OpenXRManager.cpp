#include "OpenXRManager.h"
#include "../Core/Log.h"

#include "../Core/Application.h"

namespace Katame
{
	
	OpenXRManager::OpenXRManager()
	{
		gfx = new Graphics( 1280, 720 );

		if (!openxr_init( "Single file OpenXR", gfx->m_Swapchain_fmt )) {
			delete gfx;
			throw std::exception( "OpenXR initialization failed." );
		}
		openxr_make_actions();
	}

	OpenXRManager::~OpenXRManager()
	{
		delete gfx;
	}
	
	bool OpenXRManager::HandSelect( int i )
	{
		if (xr_input.handSelect[i]) {
			return true;
		}
		return false;
	}

	XrPosef OpenXRManager::GetHandPos( int i )
	{
		return xr_input.handPose[i];
	}

	const XrPosef OpenXRManager::GetPoseIdentity()
	{
		return xr_pose_identity;
	}

	input_state_t OpenXRManager::GetInput()
	{
		return xr_input;
	}

	void OpenXRManager::SwapchainDestroy( swapchain_t& swapchain )
	{
		for (uint32_t i = 0; i < swapchain.surface_data.size(); i++) {
			swapchain.surface_data[i].depth_view->Release();
			swapchain.surface_data[i].target_view->Release();
		}
	}

	bool OpenXRManager::openxr_init( const char* app_name, int64_t swapchain_format ) 
	{
		KM_CORE_INFO( "Initializing OpenXR.." );
		// OpenXR will fail to initialize if we ask for an extension that OpenXR
		// can't provide! So we need to check our all extensions before 
		// initializing OpenXR with them. Note that even if the extension is 
		// present, it's still possible you may not be able to use it. For 
		// example: the hand tracking extension may be present, but the hand
		// sensor might not be plugged in or turned on. There are often 
		// additional checks that should be made before using certain features!
		std::vector<const char*> use_extensions;
		const char* ask_extensions[] = {
			XR_KHR_D3D11_ENABLE_EXTENSION_NAME, // Use Direct3D11 for rendering
			XR_EXT_DEBUG_UTILS_EXTENSION_NAME,  // Debug utils for extra info
		};

		// We'll get a list of extensions that OpenXR provides using this 
		// enumerate pattern. OpenXR often uses a two-call enumeration pattern 
		// where the first call will tell you how much memory to allocate, and
		// the second call will provide you with the actual data!
		uint32_t ext_count = 0;
		xrEnumerateInstanceExtensionProperties( nullptr, 0, &ext_count, nullptr );
		std::vector<XrExtensionProperties> xr_exts( ext_count, { XR_TYPE_EXTENSION_PROPERTIES } );
		xrEnumerateInstanceExtensionProperties( nullptr, ext_count, &ext_count, xr_exts.data() );

		printf( "OpenXR extensions available:\n" );
		for (size_t i = 0; i < xr_exts.size(); i++) {
			printf( "- %s\n", xr_exts[i].extensionName );

			// Check if we're asking for this extensions, and add it to our use 
			// list!
			for (int32_t ask = 0; ask < _countof( ask_extensions ); ask++) {
				if (strcmp( ask_extensions[ask], xr_exts[i].extensionName ) == 0) {
					use_extensions.push_back( ask_extensions[ask] );
					break;
				}
			}
		}
		// If a required extension isn't present, you want to ditch out here!
		// It's possible something like your rendering API might not be provided
		// by the active runtime. APIs like OpenGL don't have universal support.
		if (!std::any_of( use_extensions.begin(), use_extensions.end(),
			[]( const char* ext ) {
				return strcmp( ext, XR_KHR_D3D11_ENABLE_EXTENSION_NAME ) == 0;
			} ))
			return false;

			// Initialize OpenXR with the extensions we've found!
			XrInstanceCreateInfo createInfo = { XR_TYPE_INSTANCE_CREATE_INFO };
			createInfo.enabledExtensionCount = use_extensions.size();
			createInfo.enabledExtensionNames = use_extensions.data();
			createInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
			strcpy_s( createInfo.applicationInfo.applicationName, app_name );
			xrCreateInstance( &createInfo, &xr_instance );

			// Check if OpenXR is on this system, if this is null here, the user 
			// needs to install an OpenXR runtime and ensure it's active!
			if (xr_instance == nullptr)
				return false;

			// Load extension methods that we'll need for this application! There's a
			// couple ways to do this, and this is a fairly manual one. Chek out this
			// file for another way to do it:
			// https://github.com/maluoi/StereoKit/blob/master/StereoKitC/systems/platform/openxr_extensions.h
			xrGetInstanceProcAddr( xr_instance, "xrCreateDebugUtilsMessengerEXT", (PFN_xrVoidFunction*)(&ext_xrCreateDebugUtilsMessengerEXT) );
			xrGetInstanceProcAddr( xr_instance, "xrDestroyDebugUtilsMessengerEXT", (PFN_xrVoidFunction*)(&ext_xrDestroyDebugUtilsMessengerEXT) );
			xrGetInstanceProcAddr( xr_instance, "xrGetD3D11GraphicsRequirementsKHR", (PFN_xrVoidFunction*)(&ext_xrGetD3D11GraphicsRequirementsKHR) );

			// Set up a really verbose debug log! Great for dev, but turn this off or
			// down for final builds. WMR doesn't produce much output here, but it
			// may be more useful for other runtimes?
			// Here's some extra information about the message types and severities:
			// https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#debug-message-categorization
			XrDebugUtilsMessengerCreateInfoEXT debug_info = { XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
			debug_info.messageTypes =
				XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
				XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT;
			debug_info.messageSeverities =
				XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
				XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debug_info.userCallback = []( XrDebugUtilsMessageSeverityFlagsEXT severity, XrDebugUtilsMessageTypeFlagsEXT types, const XrDebugUtilsMessengerCallbackDataEXT* msg, void* user_data ) {
				// Print the debug message we got! There's a bunch more info we could
				// add here too, but this is a pretty good start, and you can always
				// add a breakpoint this line!
				printf( "%s: %s\n", msg->functionName, msg->message );

				// Output to debug window
				char text[512];
				sprintf_s( text, "%s: %s", msg->functionName, msg->message );
				OutputDebugStringA( text );

				// Returning XR_TRUE here will force the calling function to fail
				return (XrBool32)XR_FALSE;
			};
			// Start up the debug utils!
			if (ext_xrCreateDebugUtilsMessengerEXT)
				ext_xrCreateDebugUtilsMessengerEXT( xr_instance, &debug_info, &xr_debug );

			// Request a form factor from the device (HMD, Handheld, etc.)
			XrSystemGetInfo systemInfo = { XR_TYPE_SYSTEM_GET_INFO };
			systemInfo.formFactor = app_config_form;
			xrGetSystem( xr_instance, &systemInfo, &xr_system_id );

			// Check what blend mode is valid for this device (opaque vs transparent displays)
			// We'll just take the first one available!
			uint32_t blend_count = 0;
			xrEnumerateEnvironmentBlendModes( xr_instance, xr_system_id, app_config_view, 1, &blend_count, &xr_blend );

			// OpenXR wants to ensure apps are using the correct graphics card, so this MUST be called 
			// before xrCreateSession. This is crucial on devices that have multiple graphics cards, 
			// like laptops with integrated graphics chips in addition to dedicated graphics cards.
			XrGraphicsRequirementsD3D11KHR requirement = { XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR };
			ext_xrGetD3D11GraphicsRequirementsKHR( xr_instance, xr_system_id, &requirement );
			if (!gfx->Init( requirement.adapterLuid ))
				return false;

			// A session represents this application's desire to display things! This is where we hook up our graphics API.
			// This does not start the session, for that, you'll need a call to xrBeginSession, which we do in openxr_poll_events
			XrGraphicsBindingD3D11KHR binding = { XR_TYPE_GRAPHICS_BINDING_D3D11_KHR };
			binding.device = gfx->m_Device;
			XrSessionCreateInfo sessionInfo = { XR_TYPE_SESSION_CREATE_INFO };
			sessionInfo.next = &binding;
			sessionInfo.systemId = xr_system_id;
			xrCreateSession( xr_instance, &sessionInfo, &xr_session );

			// Unable to start a session, may not have an MR device attached or ready
			if (xr_session == nullptr)
				return false;

			// OpenXR uses a couple different types of reference frames for positioning content, we need to choose one for
			// displaying our content! STAGE would be relative to the center of your guardian system's bounds, and LOCAL
			// would be relative to your device's starting location. HoloLens doesn't have a STAGE, so we'll use LOCAL.
			XrReferenceSpaceCreateInfo ref_space = { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
			ref_space.poseInReferenceSpace = xr_pose_identity;
			ref_space.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
			xrCreateReferenceSpace( xr_session, &ref_space, &xr_app_space );

			// Now we need to find all the viewpoints we need to take care of! For a stereo headset, this should be 2.
			// Similarly, for an AR phone, we'll need 1, and a VR cave could have 6, or even 12!
			uint32_t view_count = 0;
			xrEnumerateViewConfigurationViews( xr_instance, xr_system_id, app_config_view, 0, &view_count, nullptr );
			xr_config_views.resize( view_count, { XR_TYPE_VIEW_CONFIGURATION_VIEW } );
			xr_views.resize( view_count, { XR_TYPE_VIEW } );
			xrEnumerateViewConfigurationViews( xr_instance, xr_system_id, app_config_view, view_count, &view_count, xr_config_views.data() );
			for (uint32_t i = 0; i < view_count; i++) {
				// Create a swapchain for this viewpoint! A swapchain is a set of texture buffers used for displaying to screen,
				// typically this is a backbuffer and a front buffer, one for rendering data to, and one for displaying on-screen.
				// A note about swapchain image format here! OpenXR doesn't create a concrete image format for the texture, like 
				// DXGI_FORMAT_R8G8B8A8_UNORM. Instead, it switches to the TYPELESS variant of the provided texture format, like 
				// DXGI_FORMAT_R8G8B8A8_TYPELESS. When creating an ID3D11RenderTargetView for the swapchain texture, we must specify
				// a concrete type like DXGI_FORMAT_R8G8B8A8_UNORM, as attempting to create a TYPELESS view will throw errors, so 
				// we do need to store the format separately and remember it later.
				XrViewConfigurationView& view = xr_config_views[i];
				XrSwapchainCreateInfo    swapchain_info = { XR_TYPE_SWAPCHAIN_CREATE_INFO };
				XrSwapchain              handle;
				swapchain_info.arraySize = 1;
				swapchain_info.mipCount = 1;
				swapchain_info.faceCount = 1;
				swapchain_info.format = swapchain_format;
				swapchain_info.width = view.recommendedImageRectWidth;
				swapchain_info.height = view.recommendedImageRectHeight;
				swapchain_info.sampleCount = view.recommendedSwapchainSampleCount;
				swapchain_info.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
				xrCreateSwapchain( xr_session, &swapchain_info, &handle );

				// Find out how many textures were generated for the swapchain
				uint32_t surface_count = 0;
				xrEnumerateSwapchainImages( handle, 0, &surface_count, nullptr );

				// We'll want to track our own information about the swapchain, so we can draw stuff onto it! We'll also create
				// a depth buffer for each generated texture here as well with make_surfacedata.
				swapchain_t swapchain = {};
				swapchain.width = swapchain_info.width;
				swapchain.height = swapchain_info.height;
				swapchain.handle = handle;
				swapchain.surface_images.resize( surface_count, { XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR } );
				swapchain.surface_data.resize( surface_count );
				xrEnumerateSwapchainImages( swapchain.handle, surface_count, &surface_count, (XrSwapchainImageBaseHeader*)swapchain.surface_images.data() );
				for (uint32_t i = 0; i < surface_count; i++) {

					// Get information about the swapchain image that OpenXR made for us!
					XrSwapchainImageD3D11KHR& d3d_swapchain_img = (XrSwapchainImageD3D11KHR&)swapchain.surface_images[i];
					swapchain.surface_data[i] = gfx->MakeSurfaceData( d3d_swapchain_img.texture );
				}
				xr_swapchains.push_back( swapchain );
			}

			KM_CORE_INFO( "Initialized OpenXR!" );
			return true;
	}

	void OpenXRManager::openxr_make_actions() 
	{
		XrActionSetCreateInfo actionset_info = { XR_TYPE_ACTION_SET_CREATE_INFO };
		strcpy_s( actionset_info.actionSetName, "gameplay" );
		strcpy_s( actionset_info.localizedActionSetName, "Gameplay" );
		xrCreateActionSet( xr_instance, &actionset_info, &xr_input.actionSet );
		xrStringToPath( xr_instance, "/user/hand/left", &xr_input.handSubactionPath[0] );
		xrStringToPath( xr_instance, "/user/hand/right", &xr_input.handSubactionPath[1] );

		// Create an action to track the position and orientation of the hands! This is
		// the controller location, or the center of the palms for actual hands.
		XrActionCreateInfo action_info = { XR_TYPE_ACTION_CREATE_INFO };
		action_info.countSubactionPaths = _countof( xr_input.handSubactionPath );
		action_info.subactionPaths = xr_input.handSubactionPath;
		action_info.actionType = XR_ACTION_TYPE_POSE_INPUT;
		strcpy_s( action_info.actionName, "hand_pose" );
		strcpy_s( action_info.localizedActionName, "Hand Pose" );
		xrCreateAction( xr_input.actionSet, &action_info, &xr_input.poseAction );

		// Create an action for listening to the select action! This is primary trigger
		// on controllers, and an airtap on HoloLens
		action_info.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
		strcpy_s( action_info.actionName, "select" );
		strcpy_s( action_info.localizedActionName, "Select" );
		xrCreateAction( xr_input.actionSet, &action_info, &xr_input.selectAction );

		// Bind the actions we just created to specific locations on the Khronos simple_controller
		// definition! These are labeled as 'suggested' because they may be overridden by the runtime
		// preferences. For example, if the runtime allows you to remap buttons, or provides input
		// accessibility settings.
		XrPath profile_path;
		XrPath pose_path[2];
		XrPath select_path[2];
		xrStringToPath( xr_instance, "/user/hand/left/input/grip/pose", &pose_path[0] );
		xrStringToPath( xr_instance, "/user/hand/right/input/grip/pose", &pose_path[1] );
		xrStringToPath( xr_instance, "/user/hand/left/input/select/click", &select_path[0] );
		xrStringToPath( xr_instance, "/user/hand/right/input/select/click", &select_path[1] );
		xrStringToPath( xr_instance, "/interaction_profiles/khr/simple_controller", &profile_path );
		XrActionSuggestedBinding bindings[] = {
			{ xr_input.poseAction,   pose_path[0]   },
			{ xr_input.poseAction,   pose_path[1]   },
			{ xr_input.selectAction, select_path[0] },
			{ xr_input.selectAction, select_path[1] }, };
		XrInteractionProfileSuggestedBinding suggested_binds = { XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
		suggested_binds.interactionProfile = profile_path;
		suggested_binds.suggestedBindings = &bindings[0];
		suggested_binds.countSuggestedBindings = _countof( bindings );
		xrSuggestInteractionProfileBindings( xr_instance, &suggested_binds );

		// Create frames of reference for the pose actions
		for (int32_t i = 0; i < 2; i++) {
			XrActionSpaceCreateInfo action_space_info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
			action_space_info.action = xr_input.poseAction;
			action_space_info.poseInActionSpace = xr_pose_identity;
			action_space_info.subactionPath = xr_input.handSubactionPath[i];
			xrCreateActionSpace( xr_session, &action_space_info, &xr_input.handSpace[i] );
		}

		// Attach the action set we just made to the session
		XrSessionActionSetsAttachInfo attach_info = { XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
		attach_info.countActionSets = 1;
		attach_info.actionSets = &xr_input.actionSet;
		xrAttachSessionActionSets( xr_session, &attach_info );
	}

	void OpenXRManager::openxr_shutdown() {
		// We used a graphics API to initialize the swapchain data, so we'll
		// give it a chance to release anythig here!
		for (int32_t i = 0; i < xr_swapchains.size(); i++) {
			xrDestroySwapchain( xr_swapchains[i].handle );
			SwapchainDestroy( xr_swapchains[i] );
		}
		xr_swapchains.clear();

		// Release all the other OpenXR resources that we've created!
		// What gets allocated, must get deallocated!
		if (xr_input.actionSet != XR_NULL_HANDLE) {
			if (xr_input.handSpace[0] != XR_NULL_HANDLE) xrDestroySpace( xr_input.handSpace[0] );
			if (xr_input.handSpace[1] != XR_NULL_HANDLE) xrDestroySpace( xr_input.handSpace[1] );
			xrDestroyActionSet( xr_input.actionSet );
		}
		if (xr_app_space != XR_NULL_HANDLE) xrDestroySpace( xr_app_space );
		if (xr_session != XR_NULL_HANDLE) xrDestroySession( xr_session );
		if (xr_debug != XR_NULL_HANDLE) ext_xrDestroyDebugUtilsMessengerEXT( xr_debug );
		if (xr_instance != XR_NULL_HANDLE) xrDestroyInstance( xr_instance );
	}

	void OpenXRManager::openxr_poll_events( bool& m_Running, bool& xr_running ) {
		m_Running = true;

		XrEventDataBuffer event_buffer = { XR_TYPE_EVENT_DATA_BUFFER };

		while (xrPollEvent( xr_instance, &event_buffer ) == XR_SUCCESS) {
			switch (event_buffer.type) 
			{
				case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: 
				{
					XrEventDataSessionStateChanged* changed = (XrEventDataSessionStateChanged*)&event_buffer;
					xr_session_state = changed->state;

					// Session state change is where we can begin and end sessions, as well as find quit messages!
					switch (xr_session_state) 
					{
						case XR_SESSION_STATE_READY: {
							XrSessionBeginInfo begin_info = { XR_TYPE_SESSION_BEGIN_INFO };
							begin_info.primaryViewConfigurationType = app_config_view;
							xrBeginSession( xr_session, &begin_info );
							xr_running = true;
						} break;
						case XR_SESSION_STATE_STOPPING: {
							xr_running = false;
							xrEndSession( xr_session );
						} break;
						case XR_SESSION_STATE_EXITING:      m_Running = false;              break;
						case XR_SESSION_STATE_LOSS_PENDING: m_Running = false;              break;
					}
				} 
				break;
				case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: m_Running = false; return;
			}
			event_buffer = { XR_TYPE_EVENT_DATA_BUFFER };
		}
	}

	void OpenXRManager::openxr_poll_actions() 
	{
		if (xr_session_state != XR_SESSION_STATE_FOCUSED)
			return;

		// Update our action set with up-to-date input data!
		XrActiveActionSet action_set = { };
		action_set.actionSet = xr_input.actionSet;
		action_set.subactionPath = XR_NULL_PATH;

		XrActionsSyncInfo sync_info = { XR_TYPE_ACTIONS_SYNC_INFO };
		sync_info.countActiveActionSets = 1;
		sync_info.activeActionSets = &action_set;

		xrSyncActions( xr_session, &sync_info );

		// Now we'll get the current states of our actions, and store them for later use
		for (uint32_t hand = 0; hand < 2; hand++) {
			XrActionStateGetInfo get_info = { XR_TYPE_ACTION_STATE_GET_INFO };
			get_info.subactionPath = xr_input.handSubactionPath[hand];

			XrActionStatePose pose_state = { XR_TYPE_ACTION_STATE_POSE };
			get_info.action = xr_input.poseAction;
			xrGetActionStatePose( xr_session, &get_info, &pose_state );
			xr_input.renderHand[hand] = pose_state.isActive;

			// Events come with a timestamp
			XrActionStateBoolean select_state = { XR_TYPE_ACTION_STATE_BOOLEAN };
			get_info.action = xr_input.selectAction;
			xrGetActionStateBoolean( xr_session, &get_info, &select_state );
			xr_input.handSelect[hand] = select_state.currentState && select_state.changedSinceLastSync;

			// If we have a select event, update the hand pose to match the event's timestamp
			if (xr_input.handSelect[hand]) {
				XrSpaceLocation space_location = { XR_TYPE_SPACE_LOCATION };
				XrResult        res = xrLocateSpace( xr_input.handSpace[hand], xr_app_space, select_state.lastChangeTime, &space_location );
				if (XR_UNQUALIFIED_SUCCESS( res ) &&
					(space_location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0 &&
					(space_location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0) {
					xr_input.handPose[hand] = space_location.pose;
				}
			}
		}
	}

	void OpenXRManager::openxr_poll_predicted( XrTime predicted_time ) 
	{
		if (xr_session_state != XR_SESSION_STATE_FOCUSED)
			return;

		// Update hand position based on the predicted time of when the frame will be rendered! This 
		// should result in a more accurate location, and reduce perceived lag.
		for (size_t i = 0; i < 2; i++) {
			if (!xr_input.renderHand[i])
				continue;
			XrSpaceLocation spaceRelation = { XR_TYPE_SPACE_LOCATION };
			XrResult        res = xrLocateSpace( xr_input.handSpace[i], xr_app_space, predicted_time, &spaceRelation );
			if (XR_UNQUALIFIED_SUCCESS( res ) &&
				(spaceRelation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0 &&
				(spaceRelation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0) {
				xr_input.handPose[i] = spaceRelation.pose;
			}
		}
	}

	void OpenXRManager::openxr_render_frame( Application* app ) 
	{
		// Block until the previous frame is finished displaying, and is ready for another one.
		// Also returns a prediction of when the next frame will be displayed, for use with predicting
		// locations of controllers, viewpoints, etc.
		XrFrameState frame_state = { XR_TYPE_FRAME_STATE };
		xrWaitFrame( xr_session, nullptr, &frame_state );
		// Must be called before any rendering is done! This can return some interesting flags, like 
		// XR_SESSION_VISIBILITY_UNAVAILABLE, which means we could skip rendering this frame and call
		// xrEndFrame right away.
		xrBeginFrame( xr_session, nullptr );

		// Execute any code that's dependant on the predicted time, such as updating the location of
		// controller models.
		openxr_poll_predicted( frame_state.predictedDisplayTime );
		//app_update_predicted();

		// If the session is active, lets render our layer in the compositor!
		XrCompositionLayerBaseHeader* layer = nullptr;
		XrCompositionLayerProjection             layer_proj = { XR_TYPE_COMPOSITION_LAYER_PROJECTION };
		std::vector<XrCompositionLayerProjectionView> views;
		bool session_active = xr_session_state == XR_SESSION_STATE_VISIBLE || xr_session_state == XR_SESSION_STATE_FOCUSED;
		if (session_active && openxr_render_layer( frame_state.predictedDisplayTime, views, layer_proj, app )) {
			layer = (XrCompositionLayerBaseHeader*)&layer_proj;
		}

		// We're finished with rendering our layer, so send it off for display!
		XrFrameEndInfo end_info{ XR_TYPE_FRAME_END_INFO };
		end_info.displayTime = frame_state.predictedDisplayTime;
		end_info.environmentBlendMode = xr_blend;
		end_info.layerCount = layer == nullptr ? 0 : 1;
		end_info.layers = &layer;
		xrEndFrame( xr_session, &end_info );
	}

	bool OpenXRManager::openxr_render_layer( XrTime predictedTime, std::vector<XrCompositionLayerProjectionView>& views, XrCompositionLayerProjection& layer, Application* app ) 
	{

		// Find the state and location of each viewpoint at the predicted time
		uint32_t         view_count = 0;
		XrViewState      view_state = { XR_TYPE_VIEW_STATE };
		XrViewLocateInfo locate_info = { XR_TYPE_VIEW_LOCATE_INFO };
		locate_info.viewConfigurationType = app_config_view;
		locate_info.displayTime = predictedTime;
		locate_info.space = xr_app_space;
		xrLocateViews( xr_session, &locate_info, &view_state, (uint32_t)xr_views.size(), &view_count, xr_views.data() );
		views.resize( view_count );

		// And now we'll iterate through each viewpoint, and render it!
		for (uint32_t i = 0; i < view_count; i++) {

			// We need to ask which swapchain image to use for rendering! Which one will we get?
			// Who knows! It's up to the runtime to decide.
			uint32_t                    img_id;
			XrSwapchainImageAcquireInfo acquire_info = { XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
			xrAcquireSwapchainImage( xr_swapchains[i].handle, &acquire_info, &img_id );

			// Wait until the image is available to render to. The compositor could still be
			// reading from it.
			XrSwapchainImageWaitInfo wait_info = { XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
			wait_info.timeout = XR_INFINITE_DURATION;
			xrWaitSwapchainImage( xr_swapchains[i].handle, &wait_info );

			// Set up our rendering information for the viewpoint we're using right now!
			views[i] = { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW };
			views[i].pose = xr_views[i].pose;
			views[i].fov = xr_views[i].fov;
			views[i].subImage.swapchain = xr_swapchains[i].handle;
			views[i].subImage.imageRect.offset = { 0, 0 };
			views[i].subImage.imageRect.extent = { xr_swapchains[i].width, xr_swapchains[i].height };

			// Call the rendering callback with our view and swapchain info
			gfx->RenderLayer( (float)views[i].subImage.imageRect.offset.x, (float)views[i].subImage.imageRect.offset.y, (float)views[i].subImage.imageRect.extent.width, (float)views[i].subImage.imageRect.extent.width, xr_swapchains[i].surface_data[img_id] );
			app->Draw( views[i] );

			// And tell OpenXR we're done with rendering to this one!
			XrSwapchainImageReleaseInfo release_info = { XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
			xrReleaseSwapchainImage( xr_swapchains[i].handle, &release_info );
		}

		layer.space = xr_app_space;
		layer.viewCount = (uint32_t)views.size();
		layer.views = views.data();
		return true;
	}

}