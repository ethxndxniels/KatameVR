#include "XREventHandler.h"

namespace Katame
{
	XrResult XREventHandler::m_LastCallResult = XR_SUCCESS;
	std::vector< XRCallback* > XREventHandler::m_Callbacks = {};

	inline void XREventHandler::RegisterCallback( XRCallback* pXRCallback )
	{
		m_Callbacks.push_back( pXRCallback );
	}

	inline void XREventHandler::DeregisterCallback( XRCallback* pXRCallback )
	{
		m_Callbacks.erase( std::remove( m_Callbacks.begin(), m_Callbacks.end(), pXRCallback ), m_Callbacks.end() );
		m_Callbacks.shrink_to_fit();
	}

	std::vector<XRCallback*> XREventHandler::GetCallbacks()
	{
		return m_Callbacks;
	}

}