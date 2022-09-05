#pragma once

// Tell OpenXR what platform code we'll be using
#define XR_USE_PLATFORM_WIN32
#define XR_USE_GRAPHICS_API_D3D11

#include <d3d11.h>
#include <directxmath.h> // Matrix math functions and objects
#include <d3dcompiler.h> // For compiling shaders! D3DCompile
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include "../Graphics/Graphics.h"

#include <map>

namespace Katame
{
    struct Swapchain 
    {
        XrSwapchain handle;
        int32_t width;
        int32_t height;
    };

    namespace Side
    {
        const int LEFT = 0;
        const int RIGHT = 1;
        const int COUNT = 2;
    };

    struct InputState 
    {
        XrActionSet actionSet{ XR_NULL_HANDLE };
        XrAction grabAction{ XR_NULL_HANDLE };
        XrAction poseAction{ XR_NULL_HANDLE };
        XrAction vibrateAction{ XR_NULL_HANDLE };
        XrAction quitAction{ XR_NULL_HANDLE };
        std::array<XrPath, Side::COUNT> handSubactionPath;
        std::array<XrSpace, Side::COUNT> handSpace;
        std::array<float, Side::COUNT> handScale = { {1.0f, 1.0f} };
        std::array<XrBool32, Side::COUNT> handActive;
    };

	class XRCore
	{
	public:
		void CreateInstance();
	private:
		void LogLayersAndExtensions();
		void CreateInstanceInternal();
		void LogInstanceInfo();
	private:
        //const std::shared_ptr<const Options> m_options;
        //std::shared_ptr<IPlatformPlugin> m_platformPlugin;
       // std::shared_ptr<IGraphicsPlugin> m_graphicsPlugin;
        XrInstance m_Instance{ XR_NULL_HANDLE };
        XrSession m_Session{ XR_NULL_HANDLE };
        XrSpace m_Space{ XR_NULL_HANDLE };
        XrSystemId m_SystemId{ XR_NULL_SYSTEM_ID };

        std::vector<XrViewConfigurationView> m_configViews;
        std::vector<Swapchain> m_swapchains;
        std::map<XrSwapchain, std::vector<XrSwapchainImageBaseHeader*>> m_swapchainImages;
        std::vector<XrView> m_views;
        int64_t m_colorSwapchainFormat{ -1 };

        std::vector<XrSpace> m_visualizedSpaces;

        // Application's current lifecycle state according to the runtime
        XrSessionState m_sessionState{ XR_SESSION_STATE_UNKNOWN };
        bool m_sessionRunning{ false };

        XrEventDataBuffer m_eventDataBuffer;
        InputState m_Input;

        const std::set<XrEnvironmentBlendMode> m_acceptableBlendModes;
	};
}