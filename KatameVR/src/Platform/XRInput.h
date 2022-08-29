#pragma once

#include "OpenXRManager.h"
#include <map>
#include <memory>

namespace Katame
{
	class XRInput
	{
	public:
		static std::vector< XrActionSet > ActionSets();
		static std::vector< XrActiveActionSet > ActiveActionSets();
		static XrActionSet CreateActionSet( const char* pName, const char* pLocalizedName, uint32_t nPriority );
		static XrAction CreateAction( XrActionSet xrActionSet, const char* pName, const char* pLocalizedName, XrActionType xrActionType, uint32_t nFilterCount, XrPath* xrFilters );
		static XrResult StringToXrPath( const char* sString, XrPath* xrPath );
		static XrResult CreateInputPath( const char* sControllerPath, const char* sComponentPath, XrPath* xrPath );
		static XrActionSuggestedBinding CreateActionBinding( XrAction xrAction, const char* sControllerPath, const char* sComponentPath, std::vector< XrActionSuggestedBinding >* vActionBindings );
		static XrResult SuggestActionBindings( std::vector< XrActionSuggestedBinding >* vActionBindings, const char* sInteractionProfilePath );
		static void ActivateActionSet( XrActionSet xrActionSet, XrPath xrFilter = XR_NULL_PATH );
		static XrResult SyncActiveActionSetsData();
		static XrResult GetActionPose( XrAction xrAction, XrTime xrTime, XrSpaceLocation* xrLocation );
		static XrResult GetActionStateBoolean( XrAction xrAction, XrActionStateBoolean* xrActionState );
		static XrResult GetActionStateFloat( XrAction xrAction, XrActionStateFloat* xrActionState );
		static XrResult GetActionStateVector2f( XrAction xrAction, XrActionStateVector2f* xrActionState );
		static XrResult GetActionStatePose( XrAction xrAction, XrActionStatePose* xrActionState );
		static XrResult GenerateHaptic
		(
			XrAction xrAction,
			uint64_t nDuration = XR_MIN_HAPTIC_DURATION,
			float fAmplitude = 0.5f,
			float fFrequency = XR_FREQUENCY_UNSPECIFIED
		);
	private:
		static XrResult m_LastCallResult;
		static std::vector< XrActionSet > m_vActionSets;
		static std::vector< XrActiveActionSet > m_vActiveActionSets;
		static std::vector< XrAction > m_vActions;
		static std::map< XrAction, XrSpace > m_mapActionSpace;
	};
}