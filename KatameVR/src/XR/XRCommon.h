#pragma once

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <functional>

#ifndef XR_USE_PLATFORM_WIN32
	#define XR_USE_PLATFORM_WIN32
#endif
#ifndef XR_USE_GRAPHICS_API_D3D11
	#define XR_USE_GRAPHICS_API_D3D11
	#include <d3d11.h>
	#include <d3dcompiler.h>
#endif

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <openxr/openxr_reflection.h>

namespace Katame
{
	enum EXRTrackingSpace
	{
		TRACKING_ROOMSCALE = 0,
		TRACKING_SEATED = 1
	};
	enum EXREye
	{
		EYE_LEFT = 0,
		EYE_RIGHT = 1
	};
	enum EXRInstanceExtension
	{
		EXT_INSTANCE_VISIBILITY_MASK = 0,
		EXT_INSTANCE_HAND_TRACKING = 1
	};

	struct XREyeState
	{
		XrPosef Pose;
		XrFovf	FoV;
	};

	struct XRHMDState
	{
		XREyeState LeftEye;
		XREyeState RightEye;
		bool IsPositionTracked;
		bool IsOrientationTracked;
	};
}