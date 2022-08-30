#pragma once

#include "XRCommon.h"

namespace Katame
{
	class XREventHandler
	{
	public:
		/// Register a callback to the OpenXR Provider event system
		/// @param[in] pLogger	Pointer to the logger object
		static inline void RegisterCallback( XRCallback* pXRCallback );

		/// De-register a registered callback in the OpenXR Provider event system
		/// @param[in] pXRCallback	Pointer to callback that needs de-registering
		static inline void DeregisterCallback( XRCallback* pXRCallback );


		/// Getter for the array of callback function pointers
		/// @return		Array of callback function pointers that are currently registered in the OpenXR Provider event system
		static std::vector< XRCallback* > GetXRCallbacks();

	private:
		/// Results of the last call to the OpenXR api
		static XrResult m_LastCallResult;

		/// Array of callback function pointers that are currently registered in the OpenXR Provider event system
		static std::vector< XRCallback* > m_Callbacks;
	};
}