#pragma once

// Tell OpenXR what platform code we'll be using
#define XR_USE_PLATFORM_WIN32
#define XR_USE_GRAPHICS_API_D3D11

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <cstdint>
#include <vector>

#include "XREventHandler.h"
#include "XRGraphics.h"

namespace Katame
{
	class XRCore
	{
	public:
		static bool Init( int64_t swapchain_format );
		static XrInstance* GetInstance();
		static XrSession* GetSession();
		static XrSpace* GetSpace();
	private:
		static void OpenXRInit();
		static void EnableInstanceExtensions();
		static XrResult LoadXRSystem();
		static void WorldInit();
	private:
		static XrInstance* m_Instance;
		static XrSession* m_Session;
		static XrSpace* m_Space;
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
		static XrSystemId m_SystemId;
		static XrSystemProperties m_SystemProperties;
		static XrReferenceSpaceType m_ReferenceSpaceType;

	};
}
