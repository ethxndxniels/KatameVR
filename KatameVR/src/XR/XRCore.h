#pragma once

#include "XRCommon.h"
#include "XRGraphics.h"
#include "XRHandTracking.h"

namespace Katame
{
	class XRCore
	{
	public:
		static bool Init();
		static void PollEvents( bool& m_Running, std::function<void( XrEventDataBuffer& )> OnEvent );
		static XrInstance* GetInstance();
		static XrSession* GetSession();
		static XrSpace* GetSpace();
		static XrSystemId GetSystemID();
		static XrSessionState GetState();
	public:
		static bool IsRunning();
		static bool GetIsDepthSupported();
	private:
		static void OpenXRInit();
		static void EnableInstanceExtensions();
		static XrResult LoadXRSystem();
		static void WorldInit();
	private:
		static XrInstance* m_Instance;
		static XrSession* m_Session;
		static XrSpace* m_Space;
		static XrSystemId m_SystemId;
		static XrSessionState m_State;
	private:
		static std::vector<const char*> m_AppEnabledExtensions;
		static std::vector<XrExtensionProperties*> m_AppRequestedExtensions;
	private:
		static char* s_EngineName;
		static float f_EngineVersion;
		static char* s_AppName;
		static float f_AppVersion;
		static XrResult m_LastCallResult;
		static char* s_GraphicsExtensionName;
		static XrSystemProperties m_SystemProperties;
		static XrReferenceSpaceType m_ReferenceSpaceType;
		static bool b_IsDepthSupported;
		static bool b_Running;
	private:
		static PFN_xrGetD3D11GraphicsRequirementsKHR xrGetD3D11GraphicsRequirementsKHR;
		static PFN_xrCreateDebugUtilsMessengerEXT    xrCreateDebugUtilsMessengerEXT;
		static PFN_xrDestroyDebugUtilsMessengerEXT   xrDestroyDebugUtilsMessengerEXT;
		static XrDebugUtilsMessengerEXT xr_debug;
		static XrEnvironmentBlendMode   xr_blend;
		static const XrPosef  xr_pose_identity;
	};
}
