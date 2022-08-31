#include "XRHandTracking.h"

#include "XRCore.h"

namespace Katame
{
	bool XRHandTracking::b_IsHandTrackingActive_Left = true;
	bool XRHandTracking::b_IsHandTrackingActive_Right = true;
	bool XRHandTracking::b_GetHandJointVelocities_Left = false;
	bool XRHandTracking::b_GetHandJointVelocities_Right = false;
	XrResult XRHandTracking::m_LastCallResult = XR_SUCCESS;
	XrHandJointLocationEXT XRHandTracking::m_HandJointsData_Left[XR_HAND_JOINT_COUNT_EXT] = {};
	XrHandJointLocationEXT XRHandTracking::m_HandJointsData_Right[XR_HAND_JOINT_COUNT_EXT] = {};
	XrHandJointVelocityEXT XRHandTracking::m_HandJointVelocities_Left[XR_HAND_JOINT_COUNT_EXT] = {};
	XrHandJointVelocityEXT XRHandTracking::m_HandJointVelocities_Right[XR_HAND_JOINT_COUNT_EXT] = {};
	XrHandJointVelocitiesEXT XRHandTracking::m_Velocities_Left = { XR_TYPE_HAND_JOINT_VELOCITIES_EXT };
	XrHandJointVelocitiesEXT XRHandTracking::m_Velocities_Right = { XR_TYPE_HAND_JOINT_VELOCITIES_EXT };
	XrHandJointLocationsEXT XRHandTracking::m_Locations_Left = { XR_TYPE_HAND_JOINT_LOCATIONS_EXT };
	XrHandJointLocationsEXT XRHandTracking::m_Locations_Right = { XR_TYPE_HAND_JOINT_LOCATIONS_EXT };
	XrHandTrackerEXT XRHandTracking::m_HandTracker_Left = XR_NULL_HANDLE;
	XrHandTrackerEXT XRHandTracking::m_HandTracker_Right = XR_NULL_HANDLE;
	PFN_xrLocateHandJointsEXT XRHandTracking::m_LocateHandJointsEXT = nullptr;

	bool XRHandTracking::Init()
	{
		// Create hand trackers
		XrHandTrackerCreateInfoEXT xrHandTrackerCreateInfo{ XR_TYPE_HAND_TRACKER_CREATE_INFO_EXT };
		xrHandTrackerCreateInfo.next = nullptr;
		xrHandTrackerCreateInfo.hand = XR_HAND_LEFT_EXT;
		xrHandTrackerCreateInfo.handJointSet = XR_HAND_JOINT_SET_DEFAULT_EXT;

		PFN_xrCreateHandTrackerEXT xrCreateHandTrackerEXT = nullptr;
		m_LastCallResult = xrGetInstanceProcAddr( *XRCore::GetInstance(), "xrCreateHandTrackerEXT", (PFN_xrVoidFunction*)&xrCreateHandTrackerEXT );

		m_LastCallResult = xrCreateHandTrackerEXT( *XRCore::GetSession(), &xrHandTrackerCreateInfo, &m_HandTracker_Left );

		xrHandTrackerCreateInfo.hand = XR_HAND_RIGHT_EXT;
		m_LastCallResult = xrCreateHandTrackerEXT( *XRCore::GetSession(), &xrHandTrackerCreateInfo, &m_HandTracker_Right );

		// Setup hand joint velocities
		m_Velocities_Left.jointCount = XR_HAND_JOINT_COUNT_EXT;
		m_Velocities_Left.jointVelocities = &m_HandJointVelocities_Left[0];

		m_Velocities_Right.jointCount = XR_HAND_JOINT_COUNT_EXT;
		m_Velocities_Right.jointVelocities = &m_HandJointVelocities_Right[0];

		// Setup hand joint locations
		m_Locations_Left.jointCount = XR_HAND_JOINT_COUNT_EXT;
		m_Locations_Left.jointLocations = &m_HandJointsData_Left[0];
		m_Locations_Left.next = nullptr;

		m_Locations_Right.jointCount = XR_HAND_JOINT_COUNT_EXT;
		m_Locations_Right.jointLocations = &m_HandJointsData_Right[0];
		m_Locations_Right.next = nullptr;

		// Setup function locate hand joints function pointer
		m_LastCallResult = xrGetInstanceProcAddr( *XRCore::GetInstance(), "xrLocateHandJointsEXT", (PFN_xrVoidFunction*)&m_LocateHandJointsEXT );

		return true;
	}

}