#include "XRCore.h"

#include "../Core/Log.h"

#include "../Core/Application.h"

namespace Katame
{
	XrInstance* XRCore::m_Instance = new XrInstance{};
	XrSession* XRCore::m_Session = new XrSession{ XR_NULL_HANDLE };
	XrSpace* XRCore::m_Space = new XrSpace{ XR_NULL_HANDLE };
	XrSystemId XRCore::m_SystemId = XR_NULL_SYSTEM_ID;
	XrSessionState XRCore::m_State = { XR_SESSION_STATE_UNKNOWN };

	std::vector<const char*> XRCore::m_AppEnabledExtensions = {};
	std::vector<XrExtensionProperties*> XRCore::m_AppRequestedExtensions;

	char* XRCore::s_EngineName = (char*)"KatameVR";
	float XRCore::f_EngineVersion = 0.0;
	char* XRCore::s_AppName = (char*)"Untitled";
	float XRCore::f_AppVersion = 0.0;
	XrResult XRCore::m_LastCallResult = XR_SUCCESS;
	char* XRCore::s_GraphicsExtensionName = (char*)XR_KHR_D3D11_ENABLE_EXTENSION_NAME;
	XrSystemProperties XRCore::m_SystemProperties = {XR_TYPE_SYSTEM_PROPERTIES};
	XrReferenceSpaceType XRCore::m_ReferenceSpaceType = { XR_REFERENCE_SPACE_TYPE_LOCAL };
	bool XRCore::b_IsDepthSupported = true;
	bool XRCore::b_Running = true;

	PFN_xrGetD3D11GraphicsRequirementsKHR XRCore::xrGetD3D11GraphicsRequirementsKHR = nullptr;
	PFN_xrCreateDebugUtilsMessengerEXT    XRCore::xrCreateDebugUtilsMessengerEXT = nullptr;
	PFN_xrDestroyDebugUtilsMessengerEXT   XRCore::xrDestroyDebugUtilsMessengerEXT = nullptr;
	XrDebugUtilsMessengerEXT XRCore::xr_debug = {};
	XrEnvironmentBlendMode  XRCore::xr_blend = {};
	const XrPosef  XRCore::xr_pose_identity = { {0,0,0,1}, {0,0,0} };

	bool XRCore::Init()
	{
		KM_CORE_INFO( "Initializing OpenXR.." );;

		// Initialize OpenXR
		OpenXRInit();

		// Initialize World
		WorldInit();

		return true;
	}

	void XRCore::PollEvents( bool& m_Running, std::function<void( XrEventDataBuffer& )> OnEvent )
	{
		XrEventDataBuffer xrEvent{ XR_TYPE_EVENT_DATA_BUFFER };
		xrEvent.next = nullptr;

		m_LastCallResult = xrPollEvent( *m_Instance, &xrEvent );

		// Stop evaluating if there's no event returned or the call fails
		if (xrEvent.type == XR_TYPE_EVENT_DATA_BUFFER)
			return;

		// Execute any callbacks registered for this event
		OnEvent( xrEvent );
	}

	XrInstance* XRCore::GetInstance()
	{
		return m_Instance;
	}

	XrSession* XRCore::GetSession()
	{
		return m_Session;
	}

	XrSpace* XRCore::GetSpace()
	{
		return m_Space;
	}

	XrSystemId XRCore::GetSystemID()
	{
		return m_SystemId;
	}

	XrSessionState XRCore::GetState()
	{
		return m_State;
	}

	bool XRCore::IsRunning()
	{
		return b_Running;
	}

	bool XRCore::GetIsDepthSupported()
	{
		return b_IsDepthSupported;
	}

	void XRCore::OpenXRInit()
	{
		XrInstanceCreateInfo xrInstanceCreateInfo = { XR_TYPE_INSTANCE_CREATE_INFO };
		xrInstanceCreateInfo.applicationInfo = {};

		size_t nAppNameSize = sizeof( s_AppName ) + 1;
		if (nAppNameSize > XR_MAX_APPLICATION_NAME_SIZE)
			nAppNameSize = XR_MAX_APPLICATION_NAME_SIZE;

		strncpy_s( xrInstanceCreateInfo.applicationInfo.applicationName,s_AppName, nAppNameSize );

		size_t nEngineNameSize = sizeof( s_EngineName ) + 1;
		if (nEngineNameSize > XR_MAX_ENGINE_NAME_SIZE)
			nEngineNameSize = XR_MAX_ENGINE_NAME_SIZE;
		strncpy_s( xrInstanceCreateInfo.applicationInfo.engineName, s_EngineName, nEngineNameSize );

		xrInstanceCreateInfo.applicationInfo.applicationVersion = (uint32_t)f_AppVersion;
		xrInstanceCreateInfo.applicationInfo.engineVersion = (uint32_t)f_EngineVersion;
		xrInstanceCreateInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;

		m_LastCallResult = xrCreateInstance( &xrInstanceCreateInfo, m_Instance );;
		if (m_LastCallResult != XR_SUCCESS)
			KM_CORE_ERROR( "Failed to create instance!" );

		EnableInstanceExtensions();


		uint32_t nNumEnxtesions = m_AppEnabledExtensions.empty() ? 0 : (uint32_t)m_AppEnabledExtensions.size();

		if (nNumEnxtesions > 0)
		{
			xrInstanceCreateInfo.enabledExtensionCount = nNumEnxtesions;
			xrInstanceCreateInfo.enabledExtensionNames = m_AppEnabledExtensions.data();
		}

		KM_CORE_INFO( "..." );
		KM_CORE_INFO( "XR Instance created: Handle {} with {} extension(s) enabled", (uint64_t)m_Instance, nNumEnxtesions );
		KM_CORE_INFO(
			"Instance info: OpenXR version {}.{}.{}",
			XR_VERSION_MAJOR( XR_CURRENT_API_VERSION ),
			XR_VERSION_MINOR( XR_CURRENT_API_VERSION ),
			XR_VERSION_PATCH( XR_CURRENT_API_VERSION ) );
		KM_CORE_INFO( "Instance info: Application {} version {}", s_AppName, f_AppVersion );
		KM_CORE_INFO( "Instance info: Engine {} version {}", s_EngineName, f_EngineVersion );

		LoadXRSystem();

		XrGraphicsRequirementsD3D11KHR requirement = { XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR };
		xrGetD3D11GraphicsRequirementsKHR( *m_Instance, m_SystemId, &requirement );
		XRGraphics::Init( m_Instance, &m_SystemId, m_Session, requirement.adapterLuid );

		// Setup Graphics bindings
		XrGraphicsBindingD3D11KHR xrGraphicsBinding = { XR_TYPE_GRAPHICS_BINDING_D3D11_KHR };
		xrGraphicsBinding.device = XRGraphics::GetDevice();
		// Create Session
		XrSessionCreateInfo xrSessionCreateInfo = { XR_TYPE_SESSION_CREATE_INFO };
		xrSessionCreateInfo.next = &xrGraphicsBinding;
		xrSessionCreateInfo.systemId = m_SystemId;
		m_LastCallResult = xrCreateSession( *m_Instance, &xrSessionCreateInfo, m_Session );

		if (m_LastCallResult != XR_SUCCESS)
			KM_CORE_ERROR( "Session failed to create." );
	}

	void XRCore::EnableInstanceExtensions()
	{
		m_AppEnabledExtensions.clear();

		// Log and enable runtime's available extensions
		uint32_t nExtensionCount;
		m_LastCallResult = xrEnumerateInstanceExtensionProperties( nullptr, 0, &nExtensionCount, nullptr );;

		std::vector< XrExtensionProperties > vExtensions;
		for (uint32_t i = 0; i < nExtensionCount; ++i)
		{
			vExtensions.push_back( XrExtensionProperties{ XR_TYPE_EXTENSION_PROPERTIES, nullptr } );
		}

		m_LastCallResult =
			xrEnumerateInstanceExtensionProperties( nullptr, nExtensionCount, &nExtensionCount, vExtensions.data() );

		KM_CORE_INFO( "Runtime supports the following extensions (* = will be enabled):" );
		bool bEnable = false;
		for (uint32_t i = 0; i < nExtensionCount; ++i)
		{
			// Check for graphics api extension
			if (strcmp( XR_KHR_D3D11_ENABLE_EXTENSION_NAME, &vExtensions[i].extensionName[0] ) == 0)
			{
				// Add graphics api to the list of extensions that would be enabled when we create the openxr instance
				m_AppEnabledExtensions.push_back( XR_KHR_D3D11_ENABLE_EXTENSION_NAME );
				KM_CORE_TRACE( "*{}. {} version {}", i + 1, vExtensions[i].extensionName, vExtensions[i].extensionVersion );

				bEnable = true;
			}
			else if (strcmp( XR_EXT_DEBUG_UTILS_EXTENSION_NAME, &vExtensions[i].extensionName[0] ) == 0)
			{
				// Add depth handling to the list of extensions that would be enabled when we create the openxr instance
				m_AppEnabledExtensions.push_back( XR_EXT_DEBUG_UTILS_EXTENSION_NAME );
				KM_CORE_TRACE( "*{}. {} version {}", i + 1, vExtensions[i].extensionName, vExtensions[i].extensionVersion );

				bEnable = true;
			}
			else if (
				strcmp( XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME, &vExtensions[i].extensionName[0] ) == 0)
			{
				// Add depth handling to the list of extensions that would be enabled when we create the openxr instance
				m_AppEnabledExtensions.push_back( XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME );
				KM_CORE_TRACE( "*{}. {} version {}", i + 1, vExtensions[i].extensionName, vExtensions[i].extensionVersion );

				bEnable = true;
			}
			else
			{
				// Otherwise, check if this extension was requested by the app
				for (size_t j = 0; j < m_AppRequestedExtensions.size(); j++)
				{
					KM_CORE_INFO( "Processing extension: {} is equal to {}", m_AppRequestedExtensions[j]->extensionName, &vExtensions[i].extensionName[0] );

					if (strcmp( m_AppRequestedExtensions[j]->extensionName, &vExtensions[i].extensionName[0] ) == 0)
					{
						// Add to the list of extensions that would be enabled when we create the openxr instance
						m_AppEnabledExtensions.push_back( m_AppRequestedExtensions[j]->extensionName );
						m_AppEnabledExtensions.push_back( (char*)m_AppRequestedExtensions[j] );

						KM_CORE_INFO( "*{}. {} version {}", i + 1, vExtensions[i].extensionName, vExtensions[i].extensionVersion );

						bEnable = true;
						break;
					}
				}
			}

			if (!bEnable)
				KM_CORE_TRACE( "{}. {} version {}", i + 1, vExtensions[i].extensionName, vExtensions[i].extensionVersion );
			bEnable = false;
		}

		xrGetInstanceProcAddr( *m_Instance, "xrCreateDebugUtilsMessengerEXT", (PFN_xrVoidFunction*)(&xrCreateDebugUtilsMessengerEXT) );
		xrGetInstanceProcAddr( *m_Instance, "xrDestroyDebugUtilsMessengerEXT", (PFN_xrVoidFunction*)(&xrDestroyDebugUtilsMessengerEXT) );
		xrGetInstanceProcAddr( *m_Instance, "xrGetD3D11GraphicsRequirementsKHR", (PFN_xrVoidFunction*)(&xrGetD3D11GraphicsRequirementsKHR) );

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

			printf( "%s: %s\n", msg->functionName, msg->message );

			char text[512];
			sprintf_s( text, "%s: %s", msg->functionName, msg->message );
			OutputDebugStringA( text );

			return (XrBool32)XR_FALSE;
		};
		if (xrCreateDebugUtilsMessengerEXT)
			xrCreateDebugUtilsMessengerEXT( *m_Instance, &debug_info, &xr_debug );
	}

	XrResult XRCore::LoadXRSystem()
	{
		uint32_t blend_count = 0;
		xrEnumerateEnvironmentBlendModes( *m_Instance, m_SystemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 1, &blend_count, &xr_blend );

		// Get user's system info
		XrSystemGetInfo xrSystemGetInfo = {};
		xrSystemGetInfo.type = XR_TYPE_SYSTEM_GET_INFO;
		xrSystemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

		m_LastCallResult = xrGetSystem( *m_Instance, &xrSystemGetInfo, &m_SystemId );

		// Log user's system info
		m_LastCallResult = xrGetSystemProperties( *m_Instance, m_SystemId, &m_SystemProperties );

		KM_CORE_INFO( "Active runtime: {} (Vendor Id {}) ", m_SystemProperties.systemName, m_SystemProperties.vendorId );

		return m_LastCallResult;
	}

	void XRCore::WorldInit()
	{
		KM_CORE_INFO( "XR Session for this app successfully created (Handle {})", (uint64_t)m_Session );

		XrPosef xrPose{};
		xrPose.orientation.w = 1.f;

		XrReferenceSpaceCreateInfo xrReferenceSpaceCreateInfo{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
		xrReferenceSpaceCreateInfo.poseInReferenceSpace = xr_pose_identity;
		xrReferenceSpaceCreateInfo.referenceSpaceType = m_ReferenceSpaceType;

		m_LastCallResult = xrCreateReferenceSpace( *m_Session, &xrReferenceSpaceCreateInfo, m_Space );
		KM_CORE_INFO( "XR Reference Space for this app successfully created (Handle {})", (uint64_t)m_Space );

		for (size_t j = 0; j < m_AppEnabledExtensions.size(); j++) {
			// Hand tracking
			if (strcmp( m_AppEnabledExtensions[j], XR_EXT_HAND_TRACKING_EXTENSION_NAME) == 0)
			{
				XRHandTracking::Init();
				break;
			}
		}
	}

}