#pragma once

#include "XRCommon.h"
#include "XRCallback.h"

namespace Katame
{
	class XREventHandler
	{
	public:
		static inline void RegisterCallback( XRCallback* pXRCallback );
		static inline void DeregisterCallback( XRCallback* pXRCallback );
		static std::vector<XRCallback*> GetCallbacks();
	private:
		static XrResult m_LastCallResult;
		static std::vector<XRCallback*> m_Callbacks;
	};
}