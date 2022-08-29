#pragma once

#include "XRInput.h"
#include "../Core/Log.h"

#include <cassert>

namespace Katame
{
	XrResult XRInput::m_LastCallResult = XR_SUCCESS;

	XrActionSet XRInput::CreateActionSet( const char* pName, const char* pLocalizedName, uint32_t nPriority )
	{
		XrActionSetCreateInfo xrActionSetCreateInfo{ XR_TYPE_ACTION_SET_CREATE_INFO };
		strcpy_s( xrActionSetCreateInfo.actionSetName, XR_MAX_ACTION_SET_NAME_SIZE, pName );
		strcpy_s( xrActionSetCreateInfo.localizedActionSetName, XR_MAX_LOCALIZED_ACTION_SET_NAME_SIZE, pLocalizedName );
		xrActionSetCreateInfo.priority = nPriority;

		XrActionSet xrActionSet;
		m_LastCallResult = xrCreateActionSet( m_pXRCore->GetXRInstance(), &xrActionSetCreateInfo, &xrActionSet );

		if (m_LastCallResult == XR_SUCCESS)
			m_vActionSets.push_back( xrActionSet );
		else
			KM_CORE_ERROR( "Unable to create action set {}. Runtime returned {}. Action set names should only contain lower ASCII characters, numbers, dash, period or forward slash",
				pName, m_LastCallResult );

		return xrActionSet;
	}

	XrAction XRInput::CreateAction( XrActionSet xrActionSet, const char* pName, const char* pLocalizedName,
		XrActionType xrActionType, uint32_t nFilterCount, XrPath* xrFilters )
	{
		//assert( xrActionSet != 0 && m_pXRCore->GetXRSession() != XR_NULL_HANDLE );

		// Create action
		XrActionCreateInfo xrActionCreateInfo{ XR_TYPE_ACTION_CREATE_INFO };
		strcpy_s( xrActionCreateInfo.actionName, XR_MAX_ACTION_SET_NAME_SIZE, pName );
		strcpy_s( xrActionCreateInfo.localizedActionName, XR_MAX_LOCALIZED_ACTION_SET_NAME_SIZE, pLocalizedName );
		xrActionCreateInfo.actionType = xrActionType;
		xrActionCreateInfo.countSubactionPaths = nFilterCount;
		xrActionCreateInfo.subactionPaths = xrFilters;

		XrAction xrAction;
		m_LastCallResult = xrCreateAction( xrActionSet, &xrActionCreateInfo, &xrAction );

		if (m_xrLastCallResult == XR_SUCCESS)
		{
			// Add action to array of created actions for this session
			m_vActions.push_back( xrAction );

			// If this is a pose action, create a corresponding action space
			if (xrActionType == XR_ACTION_TYPE_POSE_INPUT)
			{
				XrPosef xrPose{};
				xrPose.orientation.w = 1.f;

				XrActionSpaceCreateInfo xrActionSpaceCreateInfo{ XR_TYPE_ACTION_SPACE_CREATE_INFO };
				xrActionSpaceCreateInfo.action = xrAction;
				xrActionSpaceCreateInfo.poseInActionSpace = xrPose;
				xrActionSpaceCreateInfo.subactionPath = nFilterCount > 0 ? xrFilters[0] : XR_NULL_PATH;

				XrSpace xrSpace;
				m_xrLastCallResult = xrCreateActionSpace( m_pXRCore->GetXRSession(), &xrActionSpaceCreateInfo, &xrSpace );

				if (m_xrLastCallResult == XR_SUCCESS)
				{
					m_mapActionSpace.insert( std::pair< XrAction, XrSpace >( xrAction, xrSpace ) );
					m_pXRLogger->info( "Action {} created with reference space handle ({})", pName, (uint64_t)xrSpace );
				}
				else
				{
					m_pXRLogger->error( "Unable to create an action space for action {}. Result was {}", pName, XrEnumToString( m_xrLastCallResult ) );
				}

				return xrAction;
			}
		}
		else
		{
			m_pXRLogger->error( "Unable to create action {}. Runtime returned {}. Action names should only contain lower ASCII characters, numbers, dash, period or forward slash",
				pName, XrEnumToString( m_xrLastCallResult ) );

			return xrAction;
		}

		m_pXRLogger->info( "Action {} created", pName );
		return xrAction;
	}

	XrResult XRInput::StringToXrPath( const char* sString, XrPath* xrPath )
	{
		assert( sString && m_pXRCore && m_pXRCore->GetXRInstance() != XR_NULL_HANDLE );

		m_xrLastCallResult = XR_CALL_SILENT( xrStringToPath( m_pXRCore->GetXRInstance(), sString, xrPath ), m_pXRLogger );

		return m_xrLastCallResult;
	}

	XrResult XRInput::CreateInputPath( const char* sControllerPath, const char* sComponentPath, XrPath* xrPath )
	{
		assert( m_pXRCore && m_pXRCore->GetXRInstance() != XR_NULL_HANDLE );
		assert( sControllerPath && sComponentPath && xrPath );

		char sFullPath[XR_MAX_PATH_LENGTH];
		strcpy_s( sFullPath, XR_MAX_PATH_LENGTH, sControllerPath );
		strcat_s( sFullPath, XR_MAX_PATH_LENGTH, sComponentPath );

		return StringToXrPath( sFullPath, xrPath );
	}

	XrActionSuggestedBinding XRInput::CreateActionBinding( XrAction xrAction, const char* sControllerPath, const char* sComponentPath, std::vector< XrActionSuggestedBinding >* vActionBindings )
	{
		assert( xrAction != 0 );

		// Check if the component path is a full path (i.e. already has the controller path such as /user/hand/left/input/pose
		// this can happen for controllers such as Oculus touch with handed keys (e.g. X,Y keys are only for the left hand)
		bool bIsFullPath = (strstr( "/user/", sComponentPath ) != NULL);

		XrPath xrPath;
		if (bIsFullPath)
			StringToXrPath( sComponentPath, &xrPath );
		else
			CreateInputPath( sControllerPath, sComponentPath, &xrPath );

		XrActionSuggestedBinding xrActionSuggestedBinding;
		xrActionSuggestedBinding.action = xrAction;
		xrActionSuggestedBinding.binding = xrPath;

		vActionBindings->push_back( xrActionSuggestedBinding );

		return xrActionSuggestedBinding;
	}

	XrResult XRInput::SuggestActionBindings( std::vector< XrActionSuggestedBinding >* vActionBindings, const char* sInteractionProfilePath )
	{
		assert
		(
			m_pXRCore &&
			m_pXRCore->GetXRInstance() != XR_NULL_HANDLE &&
			m_pXRCore->GetXRSession() != XR_NULL_HANDLE
		);

		XrPath xrPath;
		xrStringToPath( m_pXRCore->GetXRInstance(), sInteractionProfilePath, &xrPath );

		m_xrLastCallResult = XR_ERROR_VALIDATION_FAILURE;
		if (vActionBindings->size() < 1)
		{
			m_pXRLogger->error( "No action bindings found. Create action bindings prior to calling SuggestActionBindings()" );
			return m_xrLastCallResult;
		}

		XrInteractionProfileSuggestedBinding xrInteractionProfileSuggestedBinding{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
		xrInteractionProfileSuggestedBinding.interactionProfile = xrPath;
		xrInteractionProfileSuggestedBinding.suggestedBindings = vActionBindings->data();
		xrInteractionProfileSuggestedBinding.countSuggestedBindings = (uint32_t)vActionBindings->size();

		m_xrLastCallResult = XR_CALL_SILENT( xrSuggestInteractionProfileBindings(
			m_pXRCore->GetXRInstance(), &xrInteractionProfileSuggestedBinding ), m_pXRLogger );

		if (m_xrLastCallResult != XR_SUCCESS)
			return m_xrLastCallResult;

		m_pXRLogger->info( "Interaction profile suggested to runtime: {}", sInteractionProfilePath );

		return m_xrLastCallResult;
	}

	void XRInput::ActivateActionSet( XrActionSet xrActionSet, XrPath xrFilter /*= XR_NULL_PATH */ )
	{
		assert( xrActionSet != XR_NULL_HANDLE );

		XrActiveActionSet xrActiveActionSet{ xrActionSet, xrFilter };
		m_vActiveActionSets.push_back( xrActiveActionSet );

		XrSessionActionSetsAttachInfo xrSessionActionSetsAttachInfo{ XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
		xrSessionActionSetsAttachInfo.countActionSets = (uint32_t)m_vActionSets.size();
		xrSessionActionSetsAttachInfo.actionSets = m_vActionSets.data();

		m_xrLastCallResult = XR_CALL_SILENT( xrAttachSessionActionSets( m_pXRCore->GetXRSession(), &xrSessionActionSetsAttachInfo ), m_pXRLogger );

		if (m_xrLastCallResult == XR_SUCCESS)
			m_pXRLogger->info( "{} action sets attached to the current session ({})", xrSessionActionSetsAttachInfo.countActionSets, (uint64_t)m_pXRCore->GetXRSession() );

	}

	XrResult XRInput::SyncActiveActionSetsData()
	{
		assert( m_pXRCore && m_pXRCore->GetXRSession() );

		if (m_vActiveActionSets.size() < 1)
			return XR_SUCCESS;

		XrActionsSyncInfo xrActionSyncInfo{ XR_TYPE_ACTIONS_SYNC_INFO };
		xrActionSyncInfo.countActiveActionSets = (uint32_t)m_vActiveActionSets.size();
		xrActionSyncInfo.activeActionSets = m_vActiveActionSets.data();

		m_xrLastCallResult = XR_CALL_SILENT( xrSyncActions( m_pXRCore->GetXRSession(), &xrActionSyncInfo ), m_pXRLogger );

		return m_xrLastCallResult;
	}

	XrResult XRInput::GetActionPose( XrAction xrAction, XrTime xrTime, XrSpaceLocation* xrLocation )
	{
		// Find the action space for the action
		std::map< XrAction, XrSpace >::iterator const iter = m_mapActionSpace.find( xrAction );
		if (iter != m_mapActionSpace.end())
			m_xrLastCallResult = xrLocateSpace( iter->second, m_pXRCore->GetXRSpace(), xrTime, xrLocation );
		else
			return XR_ERROR_VALIDATION_FAILURE;

		return m_xrLastCallResult;
	}

	XrResult XRInput::GetActionStateBoolean( XrAction xrAction, XrActionStateBoolean* xrActionState )
	{
		assert( xrAction != XR_NULL_HANDLE && xrActionState && m_pXRCore && m_pXRCore->GetXRSession() );

		XrActionStateGetInfo xrActionStateGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
		xrActionStateGetInfo.action = xrAction;
		m_xrLastCallResult = XR_CALL_SILENT( xrGetActionStateBoolean(
			m_pXRCore->GetXRSession(), &xrActionStateGetInfo, xrActionState ), m_pXRLogger );

		return m_xrLastCallResult;
	}

	XrResult XRInput::GetActionStateFloat( XrAction xrAction, XrActionStateFloat* xrActionState )
	{
		assert( xrAction != XR_NULL_HANDLE && xrActionState && m_pXRCore && m_pXRCore->GetXRSession() );

		XrActionStateGetInfo xrActionStateGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
		xrActionStateGetInfo.action = xrAction;
		m_xrLastCallResult = XR_CALL_SILENT( xrGetActionStateFloat( m_pXRCore->GetXRSession(), &xrActionStateGetInfo, xrActionState ), m_pXRLogger );

		return m_xrLastCallResult;
	}

	XrResult XRInput::GetActionStateVector2f( XrAction xrAction, XrActionStateVector2f* xrActionState )
	{
		assert( xrAction != XR_NULL_HANDLE && xrActionState && m_pXRCore && m_pXRCore->GetXRSession() );

		XrActionStateGetInfo xrActionStateGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
		xrActionStateGetInfo.action = xrAction;
		m_xrLastCallResult = XR_CALL_SILENT( xrGetActionStateVector2f( m_pXRCore->GetXRSession(), &xrActionStateGetInfo, xrActionState ), m_pXRLogger );

		return m_xrLastCallResult;
	}

	XrResult XRInput::GetActionStatePose( XrAction xrAction, XrActionStatePose* xrActionState )
	{
		assert( xrAction != XR_NULL_HANDLE && xrActionState && m_pXRCore && m_pXRCore->GetXRSession() );

		XrActionStateGetInfo xrActionStateGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
		xrActionStateGetInfo.action = xrAction;
		m_xrLastCallResult = XR_CALL_SILENT( xrGetActionStatePose( m_pXRCore->GetXRSession(), &xrActionStateGetInfo, xrActionState ), m_pXRLogger );

		return m_xrLastCallResult;
	}

	XrResult XRInput::GenerateHaptic(
		XrAction xrAction,
		uint64_t nDuration /*= XR_MIN_HAPTIC_DURATION */,
		float fAmplitude /*= 0.5f*/,
		float fFrequency /*= XR_FREQUENCY_UNSPECIFIED*/ )
	{
		assert( xrAction != XR_NULL_HANDLE && m_pXRCore && m_pXRCore->GetXRSession() );

		XrHapticVibration xrHapticVibration{ XR_TYPE_HAPTIC_VIBRATION };
		xrHapticVibration.duration = nDuration;
		xrHapticVibration.amplitude = fAmplitude;
		xrHapticVibration.frequency = fFrequency;

		XrHapticActionInfo xrHapticActionInfo{ XR_TYPE_HAPTIC_ACTION_INFO };
		xrHapticActionInfo.action = xrAction;

		m_xrLastCallResult = XR_CALL_SILENT(
			xrApplyHapticFeedback( m_pXRCore->GetXRSession(), &xrHapticActionInfo, (const XrHapticBaseHeader*)&xrHapticVibration ), m_pXRLogger );

		return m_xrLastCallResult;
	}

}