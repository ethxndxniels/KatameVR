#pragma once

#include "XRCommon.h"

namespace Katame
{
	class XRHandTracking
	{
	public:
		static bool Init();
		bool bPrecise;
		XrHandJointLocationsEXT* GetHandJointLocations( XrHandEXT eHand )
		{
			if (eHand == XR_HAND_LEFT_EXT)
				return &m_Locations_Left;

			return &m_Locations_Right;
		}

		XrHandJointVelocitiesEXT* GetHandJointVelocities( XrHandEXT eHand )
		{
			if (eHand == XR_HAND_LEFT_EXT)
				return &m_Velocities_Left;

			return &m_Velocities_Right;
		}
		static void LocateHandJoints( XrHandEXT eHand, XrSpace xrSpace, XrTime xrTime );
		static bool IsActive_Left()  { return b_IsHandTrackingActive_Left; }
		static void IsActive_Left( bool val ) { b_IsHandTrackingActive_Left = val; }
		static bool IsActive_Right()  { return b_IsHandTrackingActive_Right; }
		static void IsActive_Right( bool val ) { b_IsHandTrackingActive_Right = val; }
		static bool IncludeVelocities_Left()  { return b_GetHandJointVelocities_Left; }
		static void IncludeVelocities_Left( bool val ) { b_GetHandJointVelocities_Left = val; }
		static bool IncludeVelocities_Right()  { return b_GetHandJointVelocities_Right; }
		static void IncludeVelocities_Right( bool val ) { b_GetHandJointVelocities_Right = val; }
	private:
		static bool b_IsHandTrackingActive_Left;
		static bool b_IsHandTrackingActive_Right;
		static bool b_GetHandJointVelocities_Left;
		static bool b_GetHandJointVelocities_Right;
		static XrResult m_LastCallResult;
		static XrHandJointLocationEXT m_HandJointsData_Left[XR_HAND_JOINT_COUNT_EXT];
		static XrHandJointLocationEXT m_HandJointsData_Right[XR_HAND_JOINT_COUNT_EXT];
		static XrHandJointVelocityEXT m_HandJointVelocities_Left[XR_HAND_JOINT_COUNT_EXT];
		static XrHandJointVelocityEXT m_HandJointVelocities_Right[XR_HAND_JOINT_COUNT_EXT];
		static XrHandJointVelocitiesEXT m_Velocities_Left;
		static XrHandJointVelocitiesEXT m_Velocities_Right;
		static XrHandJointLocationsEXT m_Locations_Left;
		static XrHandJointLocationsEXT m_Locations_Right;
		static XrHandTrackerEXT m_HandTracker_Left;
		static XrHandTrackerEXT m_HandTracker_Right;
		static PFN_xrLocateHandJointsEXT m_LocateHandJointsEXT;
	};
}
