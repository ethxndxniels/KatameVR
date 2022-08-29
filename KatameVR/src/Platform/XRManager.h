#pragma once

#include "XRCore.h"
#include "XRInput.h"

namespace Katame
{

	class XRManager
	{
		/// OpenXR Core System
		XRCore* Core() const { return m_pXRCoreSystem; }

		/// OpenXR Render Manager
		XRRender* Render() const { return m_pXRRenderManager; }

		/// OpenXR Input Manager
		XRInput* Input() const { return m_pXRInputManager; }

	private:
		/// Pointer to the OpenXR Core System
		XRCore* m_pXRCoreSystem = nullptr;

		/// Pointer to the OpenXR Render Manager
		XRRender* m_pXRRenderManager = nullptr;

		/// Pointer to the OpenXR Input Manager
		XRInput* m_pXRInputManager = nullptr;
	};

}