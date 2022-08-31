#include "XRCore.h"

#include "../Core/Log.h"

#include "XRGraphics.h"

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
	XrReferenceSpaceType XRCore::m_ReferenceSpaceType = { XR_REFERENCE_SPACE_TYPE_STAGE };
	bool XRCore::b_IsDepthSupported = true;
	bool XRCore::b_Running = true;

	bool XRCore::Init()
	{
		KM_CORE_INFO( "Initializing OpenXR.." );;

		// Initialize OpenXR
		OpenXRInit();

		KM_CORE_INFO( "Initializing World.." );;

		// Setup world
		WorldInit();

		return true;
	}

	bool XRCore::PollEvents()
	{
		XrEventDataBuffer event_buffer = { XR_TYPE_EVENT_DATA_BUFFER };

		while (xrPollEvent( *m_Instance, &event_buffer ) == XR_SUCCESS) {
			switch (event_buffer.type)
			{
			case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
			{
				XrEventDataSessionStateChanged* changed = (XrEventDataSessionStateChanged*)&event_buffer;
				m_State = changed->state;

				// Session state change is where we can begin and end sessions, as well as find quit messages!
				switch (m_State)
				{
				case XR_SESSION_STATE_READY: {
					XrSessionBeginInfo begin_info = { XR_TYPE_SESSION_BEGIN_INFO };
					begin_info.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
					xrBeginSession( *m_Session, &begin_info );
					b_Running = true;
				} break;
				case XR_SESSION_STATE_STOPPING: {
					b_Running = false;
					xrEndSession( *m_Session );
				} break;
				case XR_SESSION_STATE_EXITING:      return false;              break;
				case XR_SESSION_STATE_LOSS_PENDING: return false;              break;
				}
			}
			break;
			case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: return false; return true;
			}
			event_buffer = { XR_TYPE_EVENT_DATA_BUFFER };
			return true;
		}
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

		EnableInstanceExtensions();
		uint32_t nNumEnxtesions = m_AppEnabledExtensions.empty() ? 0 : (uint32_t)m_AppEnabledExtensions.size();

		if (nNumEnxtesions > 0)
		{
			xrInstanceCreateInfo.enabledExtensionCount = nNumEnxtesions;
			xrInstanceCreateInfo.enabledExtensionNames = m_AppEnabledExtensions.data();
		}

		m_LastCallResult = xrCreateInstance( &xrInstanceCreateInfo, m_Instance );;
		if (m_LastCallResult != XR_SUCCESS)
			KM_CORE_ERROR( "Failed to create instance!" );

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
	}

	XrResult XRCore::LoadXRSystem()
	{
		if (m_Instance == XR_NULL_HANDLE)
		{
			std::string eMessage = "No OpenXR Instance found. Make sure to call Init first";
			KM_CORE_ERROR( "{}. Error ({})", eMessage, std::to_string( m_LastCallResult ) );
			throw std::runtime_error( eMessage );
		}

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
		XRGraphics::Init( m_Instance, &m_SystemId, m_Session );

		if (m_LastCallResult != XR_SUCCESS)
			KM_CORE_ERROR( "{} ({})", "Failed creating OpenXR Session with Error ", std::to_string( m_LastCallResult ) );
		

		KM_CORE_INFO( "XR Session for this app successfully created (Handle {})", (uint64_t)m_Session );

		XrPosef xrPose{};
		xrPose.orientation.w = 1.f;

		XrReferenceSpaceCreateInfo xrReferenceSpaceCreateInfo{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
		xrReferenceSpaceCreateInfo.poseInReferenceSpace = xrPose;
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

	void XRCore::ExecuteCallbacks( XrEventDataBuffer xrEvent )
	{
		for ( XRCallback* xrCallback : XREventHandler::GetCallbacks() )
		{
			if (xrCallback->type == xrEvent.type || xrCallback->type == XR_TYPE_EVENT_DATA_BUFFER)
				xrCallback->callback( xrEvent );
		}
	}

}