#include "XRCore.h"

#include "../Core/Log.h"

#include "XRGraphicsAPI.h"

namespace Katame
{
	XrInstance* XRCore::m_Instance = nullptr;
	XrSession* XRCore::m_Session = XR_NULL_HANDLE;

	std::vector<const char*> XRCore::m_AppEnabledExtensions = {};
	std::vector<void*> XRCore::m_AppRequestedExtensions;

	char* XRCore::s_EngineName = (char*)"KatameVR";
	float XRCore::f_EngineVersion = 0.0;
	char* XRCore::s_AppName = (char*)"Untitled";
	float XRCore::f_AppVersion = 0.0;
	XrResult XRCore::m_LastCallResult = XR_SUCCESS;
	char* XRCore::s_GraphicsExtensionName = (char*)XR_KHR_D3D11_ENABLE_EXTENSION_NAME;
	XrSystemId XRCore::m_SystemId = XR_NULL_SYSTEM_ID;
	XrSystemProperties XRCore::m_SystemProperties = {XR_TYPE_SYSTEM_PROPERTIES};
	XrReferenceSpaceType XRCore::m_ReferenceSpaceType = { XR_REFERENCE_SPACE_TYPE_STAGE };
	XrSpace XRCore::m_Space = XR_NULL_HANDLE;

	bool XRCore::Init( int64_t swapchain_format )
	{
		KM_CORE_INFO( "Initializing OpenXR.." );;

		//event handler init?

		// Initialize OpenXR
		OpenXRInit();

		// Setup world
		WorldInit();
	}

	XrInstance* XRCore::GetInstance()
	{
		return m_Instance;
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

		xrInstanceCreateInfo.applicationInfo.applicationVersion = f_AppVersion;
		xrInstanceCreateInfo.applicationInfo.engineVersion = f_EngineVersion;
		xrInstanceCreateInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;

		EnableInstanceExtensions();
		uint32_t nNumEnxtesions = m_AppEnabledExtensions.empty() ? 0 : (uint32_t)m_AppEnabledExtensions.size();

		if (nNumEnxtesions > 0)
		{
			xrInstanceCreateInfo.enabledExtensionCount = nNumEnxtesions;
			xrInstanceCreateInfo.enabledExtensionNames = m_AppEnabledExtensions.data();
		}

		m_LastCallResult = xrCreateInstance( &xrInstanceCreateInfo, m_Instance );;

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
				KM_CORE_INFO( "*{}. {} version {}", i + 1, vExtensions[i].extensionName, vExtensions[i].extensionVersion );

				bEnable = true;
			}

			// Check for depth extension
			else if (
				strcmp( XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME, &vExtensions[i].extensionName[0] ) == 0)
			{
				// Add depth handling to the list of extensions that would be enabled when we create the openxr instance
				m_AppEnabledExtensions.push_back( XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME );
				KM_CORE_INFO( "*{}. {} version {}", i + 1, vExtensions[i].extensionName, vExtensions[i].extensionVersion );

				bEnable = true;
			}
			else
			{
				// Otherwise, check if this extension was requested by the app
				for (size_t j = 0; j < m_AppRequestedExtensions.size(); j++)
				{
					//XRBaseExt* xrRequestedExtension = static_cast<XRBaseExt*>(m_vAppRequestedExtensions[j]);

					//KM_CORE_INFO( "Processing extension: {} is equal to {}", xrRequestedExtension->GetExtensionName(), &vExtensions[i].extensionName[0] );

					//if (strcmp( xrRequestedExtension->GetExtensionName(), &vExtensions[i].extensionName[0] ) == 0)
					//{
					//	// Add to the list of extensions that would be enabled when we create the openxr instance
					//	m_AppEnabledExtensions.push_back( xrRequestedExtension->GetExtensionName() );

					//	xrRequestedExtension->IsActive( true );
					//	m_AppEnabledExtensions.push_back( (char*)m_AppRequestedExtensions[j] );

					//	KM_CORE_INFO( "*{}. {} version {}", i + 1, vExtensions[i].extensionName, vExtensions[i].extensionVersion );

					//	bEnable = true;
					//	break;
					//}
				}
			}

			if (!bEnable)
				KM_CORE_INFO( "{}. {} version {}", i + 1, vExtensions[i].extensionName, vExtensions[i].extensionVersion );
			bEnable = false;
		}
	}

	XrResult XRCore::LoadXRSystem()
	{
		if (m_Instance == XR_NULL_HANDLE)
		{
			std::string eMessage = "No OpenXR Instance found. Make sure to call Init first";
			KM_CORE_INFO( "{}. Error ({})", eMessage, std::to_string( m_LastCallResult ) );
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
		XRGraphicsAPI::Init( m_Instance, &m_SystemId, m_Session, &m_LastCallResult );

		if (m_LastCallResult != XR_SUCCESS)
			KM_CORE_ERROR( "{} ({})", "Failed creating OpenXR Session with Error ", std::to_string( m_LastCallResult ) );
		

		KM_CORE_INFO( "XR Session for this app successfully created (Handle {})", (uint64_t)m_Session );

		XrPosef xrPose{};
		xrPose.orientation.w = 1.f;

		XrReferenceSpaceCreateInfo xrReferenceSpaceCreateInfo{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
		xrReferenceSpaceCreateInfo.poseInReferenceSpace = xrPose;
		xrReferenceSpaceCreateInfo.referenceSpaceType = m_ReferenceSpaceType;

		m_LastCallResult = xrCreateReferenceSpace( *m_Session, &xrReferenceSpaceCreateInfo, &m_Space );
		KM_CORE_INFO( "XR Reference Space for this app successfully created (Handle {})", (uint64_t)m_Space );

		//for	each (void* xrExtension in GetXREnabledExtensions())
		//{
		//	XRBaseExt* xrInstanceExtension = static_cast<XRBaseExt*>(xrExtension);

		//	// Hand tracking
		//	if (strcmp( xrInstanceExtension->GetExtensionName(), XR_EXT_HAND_TRACKING_EXTENSION_NAME ) == 0)
		//	{
		//		// Set the extension member
		//		m_pXRHandTracking = static_cast<XRExtHandTracking*>(xrExtension);

		//		// Initialize extension
		//		m_pXRHandTracking->Init( GetXRInstance(), GetXRSession() );
		//		break;
		//	}
		//}
	}

}