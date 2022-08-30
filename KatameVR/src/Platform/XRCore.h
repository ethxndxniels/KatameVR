#pragma once

#include "XRCommon.h"

namespace Katame
{
	class XRCore
	{
	public:
		static bool Init( int64_t swapchain_format );
		static XrInstance* GetInstance();
		static XrSession* GetSession();
		static XrSpace* GetSpace();
		static XrSystemId GetSystemID();
	public:
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
	private:
		static std::vector<const char*> m_AppEnabledExtensions;
		static std::vector<void*> m_AppRequestedExtensions;
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
	};
}
