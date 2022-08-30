#include "XRRender.h"

#include "../Core/Log.h"

namespace Katame
{
	XRHMDState* XRRender::m_HMDState = nullptr;
	XrResult XRRender::m_LastCallResult = XR_SUCCESS;
	XrTime  XRRender::m_PredictedDisplayTime;
	XrDuration  XRRender::m_PredictedDisplayPeriod;

	bool XRRender::m_bDepthHandling = false;
	uint32_t XRRender::u_TextureArraySize = 1;
	uint32_t XRRender::u_TextureMipCount = 1;
	uint32_t XRRender::u_TextureWidth = 1920;
	uint32_t XRRender::u_TextureHeight = 1080;
	int64_t XRRender::i_TextureFormat = 0;
	int64_t XRRender::i_DepthFormat = 0;

	std::vector<XrSwapchain> XRRender::m_SwapChainsColor = {};
	std::vector<XrSwapchain> XRRender::m_SwapChainsDepth = {};
	std::vector<XrView> XRRender::m_Views = {};
	std::vector<XrViewConfigurationView> XRRender::m_ViewConfigs = {};

	const float XRRender::k_fMedianIPD = 0.063f;
	const float XRRender::k_fMinIPD = 0.04f;
	const float XRRender::k_fTau = 1.570f;

	void XRRender::Init()
	{
		// Get number of view configurations the runtime supports
		uint32_t nViewConfigTypeCount = 0;
		m_LastCallResult =
			xrEnumerateViewConfigurations( *XRCore::GetInstance(), XRCore::GetSystemID(), 0, &nViewConfigTypeCount, nullptr);

		// Retrieve all the view configuration types the runtime supports
		std::vector< XrViewConfigurationType > xrViewConfigTypes( nViewConfigTypeCount );
		m_LastCallResult =
			xrEnumerateViewConfigurations(
				*XRCore::GetInstance(), XRCore::GetSystemID(), nViewConfigTypeCount, &nViewConfigTypeCount, xrViewConfigTypes.data() );

		// Look for Stereo (VR) config. TODO: XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO (AR Support)
		bool bStereoFound = false;
		for (uint32_t i = 0; i < nViewConfigTypeCount; ++i)
		{
			if (xrViewConfigTypes[i] == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO)
			{
				bStereoFound = true;
				break;
			}
		}

		// Check if Stereo (VR) is supported by the runtime
		if (!bStereoFound)
			KM_CORE_ERROR( "Failed to create XR Render manager. The active runtime does not support VR" );

		KM_CORE_INFO( "Runtime confirms VR support amongst its {} supported view configuration(s)", nViewConfigTypeCount );

		// Get number of configuration views supported by the runtime
		uint32_t nViewCount = 0;
		m_LastCallResult = xrEnumerateViewConfigurationViews(
			*XRCore::GetInstance(), XRCore::GetSystemID(), XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &nViewCount, nullptr );

		// Retrieve configuration views from the runtime
		m_ViewConfigs.resize( nViewCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW } );
		m_LastCallResult = xrEnumerateViewConfigurationViews(
			*XRCore::GetInstance(), 
			XRCore::GetSystemID(),
			XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
			nViewCount,
			&nViewCount,
			m_ViewConfigs.data() );

		KM_CORE_INFO( "Successfully retrieved {} configuration views from the runtime. Should be two for VR (one for each eye)", nViewCount );

		// Begin XR Session
		XrSessionBeginInfo xrSessionBeginInfo = { XR_TYPE_SESSION_BEGIN_INFO };
		xrSessionBeginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
		m_LastCallResult = xrBeginSession( *XRCore::GetSession(), &xrSessionBeginInfo );

		KM_CORE_INFO( "XR Session started (Handle {})", (uint64_t)*XRCore::GetSession() );

		// Allocate xr views
		m_Views.resize( nViewCount, { XR_TYPE_VIEW } );

		// Final sanity check - view count must be 2 for VR support (one for each eye)
		if (nViewCount != k_nVRViewCount)
		{
			KM_CORE_ERROR( "Something went wrong. Runtime returned {} views for VR support, while we expected exactly 2 (one for each eye)", nViewCount );
			throw std::runtime_error( "Something went wrong. Runtime returned incorrect number of views for VR support. Expected 2 (one for each eye)" );
		}
		KM_CORE_INFO( "{} XR views successfully (one for each eye)", nViewCount );

		// Reset HMD state
		m_HMDState = new XRHMDState();
		ResetHMDState();

		// Set swapchain details
		m_bDepthHandling = XRCore::GetIsDepthSupported();

		SetSwapchainFormat( { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB }, { DXGI_FORMAT_D16_UNORM } );

		// Generate Swapchains
		GenerateSwapchains( false ); // Color textures

		if (m_bDepthHandling)
			GenerateSwapchains( true ); // Depth textures

		// Generate Swapchain images (color)
		for (uint32_t i = 0; i < m_SwapChainsColor.size(); i++)
		{
			m_LastCallResult = XRGraphics::GenerateSwapchainImages( m_SwapChainsColor[i], i, false );

			if (m_LastCallResult != XR_SUCCESS)
				KM_CORE_ERROR( "Failed to generate swapchain color buffers with error {}", m_LastCallResult );

			KM_CORE_INFO(
				"{} Swapchain color buffers generated for eye ({})", XRGraphics::GetSwapchainImageCount(i == 0 ? EYE_LEFT : EYE_RIGHT, false), i);
		}

		// Generate Swapchain images (depth)
		if (m_bDepthHandling)
		{
			for (uint32_t i = 0; i < m_SwapChainsDepth.size(); i++)
			{
				XRGraphics::GenerateSwapchainImages( m_SwapChainsDepth[i], i, true );

				if (m_LastCallResult != XR_SUCCESS)
					KM_CORE_ERROR( "Failed to generate swapchain depth buffers with error {}", m_LastCallResult );
				

				KM_CORE_INFO(
					"{} Swapchain depth buffers generated for eye ({})", XRGraphics::GetSwapchainImageCount( i == 0 ? EYE_LEFT : EYE_RIGHT, true ), i );
			}
		}
		else
		{
			KM_CORE_INFO( "Runtime does not support depth composition. No Swapchain depth buffers will be generated" );
		}

		//// Add supported extension - Visibility Mask
		//for	each (void* xrExtension in m_pXRCore->GetXREnabledExtensions())
		//{
		//	XRBaseExt* xrInstanceExtension = static_cast<XRBaseExt*>(xrExtension);

		//	if (strcmp( xrInstanceExtension->GetExtensionName(), XR_KHR_VISIBILITY_MASK_EXTENSION_NAME ) == 0)
		//	{
		//		m_pXRVisibilityMask = static_cast<XRExtVisibilityMask*>(xrExtension);

		//		m_pXRVisibilityMask->m_xrInstance = m_pXRCore->GetXRInstance();
		//		m_pXRVisibilityMask->m_xrSession = m_pXRCore->GetXRSession();
		//		break;
		//	}
		//}

		KM_CORE_INFO( "Render manager created successfully" );
	}

	void XRRender::Destroy()
	{
		// Destroy Swapchains
		DestroySwapchains( m_SwapChainsColor );
		DestroySwapchains( m_SwapChainsDepth );

		// Clear Swapchains
		m_SwapChainsColor.clear();
		m_SwapChainsDepth.clear();
	}

	void XRRender::DestroySwapchains( std::vector< XrSwapchain >& vXRSwapchains )
	{
		uint32_t nSwapChains = (uint32_t)vXRSwapchains.size();
		for (uint32_t i = 0; i < nSwapChains; i++)
		{
			if (vXRSwapchains[i] != XR_NULL_HANDLE)
			{
				m_LastCallResult = xrDestroySwapchain( vXRSwapchains[i] );
				if (m_LastCallResult != XR_SUCCESS)
					KM_CORE_ERROR( "Unable to destroy swapchain with error {}", m_LastCallResult );				
				else
					KM_CORE_INFO( "Swapchain destroyed for eye ({})", i );
			}
		}
	}

	void XRRender::ResetHMDState()
	{
		m_HMDState->LeftEye.Pose = { { 0.f, 0.f, 0.f, 1.f }, { 0.f, 0.f, 0.f } };
		m_HMDState->RightEye.Pose = { { 0.f, 0.f, 0.f, 1.f }, { 0.f, 0.f, 0.f } };

		m_HMDState->LeftEye.FoV = { -k_fTau, k_fTau, k_fTau, -k_fTau };
		m_HMDState->RightEye.FoV = { -k_fTau, k_fTau, k_fTau, -k_fTau };

		m_HMDState->IsPositionTracked = false;
		m_HMDState->IsOrientationTracked = false;
	}

	bool XRRender::ProcessXRFrame()
	{
		assert( XRCore::GetInstance() );

		// ========================================================================
		// (1) Wait for a new frame
		// ========================================================================
		XrFrameWaitInfo xrWaitFrameInfo{ XR_TYPE_FRAME_WAIT_INFO };
		XrFrameState xrFrameState{ XR_TYPE_FRAME_STATE };

		m_LastCallResult =  xrWaitFrame( *XRCore::GetSession(), &xrWaitFrameInfo, &xrFrameState );
		if (m_LastCallResult != XR_SUCCESS)
			return false;

		m_PredictedDisplayTime = xrFrameState.predictedDisplayTime;
		m_PredictedDisplayPeriod = xrFrameState.predictedDisplayPeriod;

		// ========================================================================
		// (2) Begin frame before doing any GPU work
		// ========================================================================
		XrFrameBeginInfo xrBeginFrameInfo{ XR_TYPE_FRAME_BEGIN_INFO };
		m_LastCallResult = xrBeginFrame( *XRCore::GetSession(), &xrBeginFrameInfo );
		if (m_LastCallResult != XR_SUCCESS)
			return false;

		std::vector< XrCompositionLayerBaseHeader* > xrFrameLayers;
		XrCompositionLayerProjectionView xrFrameLayerProjectionViews[k_nVRViewCount];
		XrCompositionLayerProjection xrFrameLayerProjection{ XR_TYPE_COMPOSITION_LAYER_PROJECTION };

		if (xrFrameState.shouldRender)
		{
			// ========================================================================
			// (3) Get space and time information for this frame
			// ========================================================================

			XrViewLocateInfo xrFrameSpaceTimeInfo{ XR_TYPE_VIEW_LOCATE_INFO };
			xrFrameSpaceTimeInfo.displayTime = xrFrameState.predictedDisplayTime;
			xrFrameSpaceTimeInfo.space = *XRCore::GetSpace();

			XrViewState xrFrameViewState{ XR_TYPE_VIEW_STATE };
			uint32_t nFoundViewsCount;
			m_LastCallResult = xrLocateViews(
				*XRCore::GetSession(), &xrFrameSpaceTimeInfo, &xrFrameViewState, (uint32_t)m_Views.size(), &nFoundViewsCount, m_Views.data() );

			if (m_LastCallResult != XR_SUCCESS)
				return false;

			// ========================================================================
			// (4) Grab image from swapchain and render
			// ========================================================================

			// Update HMD State
			m_HMDState->IsPositionTracked = xrFrameViewState.viewStateFlags & XR_VIEW_STATE_POSITION_TRACKED_BIT;
			m_HMDState->IsOrientationTracked = xrFrameViewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_TRACKED_BIT;

			if (xrFrameViewState.viewStateFlags & XR_VIEW_STATE_POSITION_VALID_BIT && xrFrameViewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT)
			{
				// Update hmd state
				SetHMDState( EXREye::EYE_LEFT, &(m_HMDState->LeftEye) );
				SetHMDState( EXREye::EYE_RIGHT, &(m_HMDState->RightEye) );

				// Grab the corresponding swapchain for each view location
				uint32_t nViewCount = (uint32_t)m_ViewConfigs.size();

				for (uint32_t i = 0; i < nViewCount; i++)
				{
					// ----------------------------------------------------------------
					// (a) Acquire swapchain image
					// ----------------------------------------------------------------
					const XrSwapchain xrSwapchain = m_SwapChainsColor[i];
					XrSwapchainImageAcquireInfo xrAcquireInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
					uint32_t nImageIndex;
					m_LastCallResult = xrAcquireSwapchainImage( xrSwapchain, &xrAcquireInfo, &nImageIndex );

					if (m_LastCallResult != XR_SUCCESS)
						return false;

					// ----------------------------------------------------------------
					// (b) Wait for swapchain image
					// ----------------------------------------------------------------
					XrSwapchainImageWaitInfo xrWaitInfo{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
					xrWaitInfo.timeout = XR_INFINITE_DURATION;
					m_LastCallResult = xrWaitSwapchainImage( xrSwapchain, &xrWaitInfo  );

					if (m_LastCallResult != XR_SUCCESS)
						return false;

					// ----------------------------------------------------------------
					// (c) Add projection view to swapchain image
					// ----------------------------------------------------------------

					xrFrameLayerProjectionViews[i] = { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW };
					xrFrameLayerProjectionViews[i].pose = m_Views[i].pose;
					xrFrameLayerProjectionViews[i].fov = m_Views[i].fov;
					xrFrameLayerProjectionViews[i].subImage.swapchain = xrSwapchain;
					xrFrameLayerProjectionViews[i].subImage.imageArrayIndex = 0;
					xrFrameLayerProjectionViews[i].subImage.imageRect.offset = { 0, 0 };
					xrFrameLayerProjectionViews[i].subImage.imageRect.extent = { (int32_t)u_TextureWidth, (int32_t)u_TextureHeight };

					if (m_bDepthHandling)
					{
						XrCompositionLayerDepthInfoKHR xrDepthInfo{ XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR };
						xrDepthInfo.subImage.swapchain = m_SwapChainsDepth[i];
						xrDepthInfo.subImage.imageArrayIndex = 0;
						xrDepthInfo.subImage.imageRect.offset = { 0, 0 };
						xrDepthInfo.subImage.imageRect.extent = { (int32_t)u_TextureWidth, (int32_t)u_TextureHeight };
						xrDepthInfo.minDepth = 0.0f;
						xrDepthInfo.maxDepth = 1.0f;
						xrDepthInfo.nearZ = 0.1f;
						xrDepthInfo.farZ = FLT_MAX;

						xrFrameLayerProjectionViews[i].next = &xrDepthInfo;
					}

					// ----------------------------------------------------------------
					// (d) Release swapchain image
					// ----------------------------------------------------------------
					XrSwapchainImageReleaseInfo xrSwapChainRleaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
					m_LastCallResult = xrReleaseSwapchainImage( xrSwapchain, &xrSwapChainRleaseInfo );

					if (m_LastCallResult != XR_SUCCESS)
						return false;
				}
			}

			// ----------------------------------------------------------------
			// (e) Assemble projection layers
			// ----------------------------------------------------------------
			xrFrameLayerProjection.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
			xrFrameLayerProjection.space = *XRCore::GetSpace();
			xrFrameLayerProjection.viewCount = k_nVRViewCount;
			xrFrameLayerProjection.views = xrFrameLayerProjectionViews;
			xrFrameLayers.push_back( reinterpret_cast<XrCompositionLayerBaseHeader*>(&xrFrameLayerProjection) );
		}

		// ========================================================================
		// (5) End current frame
		// ========================================================================
		XrFrameEndInfo xrEndFrameInfo{ XR_TYPE_FRAME_END_INFO };
		xrEndFrameInfo.displayTime = xrFrameState.predictedDisplayTime;
		xrEndFrameInfo.environmentBlendMode =
			XR_ENVIRONMENT_BLEND_MODE_OPAQUE; // TODO: XR_ENVIRONMENT_BLEND_MODE_ADDITIVE / XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND (AR)
		xrEndFrameInfo.layerCount = (uint32_t)xrFrameLayers.size();
		xrEndFrameInfo.layers = xrFrameLayers.data();

		m_LastCallResult = xrEndFrame( *XRCore::GetSession(), &xrEndFrameInfo );
		if (m_LastCallResult != XR_SUCCESS)
			return false;

		return true;
	}

	std::vector<XrViewConfigurationView> XRRender::GetConfigViews()
	{
		return m_ViewConfigs;
	}

	bool XRRender::GetDepthHandling()
	{
		return m_bDepthHandling;
	}

	void XRRender::SetHMDState( EXREye eEye, XREyeState* pEyeState )
	{
		assert( pEyeState );

		uint32_t nEye = (eEye == EXREye::EYE_LEFT) ? 0 : 1;

		// Update Position
		pEyeState->Pose = m_Views[nEye].pose;

		// Update Orientation
		pEyeState->Pose.orientation = m_Views[nEye].pose.orientation;

		// Update field of view
		pEyeState->FoV = m_Views[nEye].fov;
	}

	float XRRender::GetCurrentIPD()
	{
		// Get mid-point of eye positions (disregard Z)
		XrVector2f midEyePosition;
		midEyePosition.x = (m_HMDState->LeftEye.Pose.position.x + m_HMDState->RightEye.Pose.position.x) / 2.0f;
		midEyePosition.y = (m_HMDState->LeftEye.Pose.position.y + m_HMDState->RightEye.Pose.position.y) / 2.0f;

		float IPD = (std::sqrtf(
			std::powf( (midEyePosition.x - m_HMDState->LeftEye.Pose.position.x), 2 ) +
			std::powf( (midEyePosition.y - m_HMDState->LeftEye.Pose.position.y), 2 ) +
			std::powf( (midEyePosition.y - m_HMDState->LeftEye.Pose.position.y), 2 ) )) *
			2.0f;

		if (IPD < k_fMinIPD)
			return k_fMedianIPD;

		return IPD;
	}

	XrPosef XRRender::GetHMDPose()
	{
		// Get mid-point of eye positions
		XrVector3f midEyePosition = { 0 };
		midEyePosition.x = (m_HMDState->LeftEye.Pose.position.x + m_HMDState->RightEye.Pose.position.x) / 2.0f;
		midEyePosition.y = (m_HMDState->LeftEye.Pose.position.y + m_HMDState->RightEye.Pose.position.y) / 2.0f;
		midEyePosition.z = (m_HMDState->LeftEye.Pose.position.z + m_HMDState->RightEye.Pose.position.z) / 2.0f;

		return XrPosef{ m_HMDState->LeftEye.Pose.orientation, midEyePosition };
	}

	XRHMDState* XRRender::GetHMDState()
	{
		return m_HMDState;
	}



	std::vector<XrSwapchain> XRRender::GetSwapchainColor()
	{
		return m_SwapChainsColor;
	}

	std::vector<XrSwapchain> XRRender::GetSwapchainDepth()
	{
		return m_SwapChainsDepth;
	}

	XrTime XRRender::GetPredictedDisplayTime()
	{
		return m_PredictedDisplayTime;
	}

	XrDuration XRRender::GetPredictedDisplayPeriod()
	{
		return m_PredictedDisplayPeriod;
	}

	uint32_t XRRender::GetTextureArraySize()
	{
		return u_TextureArraySize;
	}

	void XRRender::SetSwapchainFormat( std::vector< int64_t > vAppTextureFormats, std::vector< int64_t > vAppDepthFormats )
	{
		assert( XRCore::GetSession() != XR_NULL_HANDLE);

		// Check number of swapchain formats supported by the runtime
		uint32_t nNumOfSupportedFormats = 0;
		m_LastCallResult = xrEnumerateSwapchainFormats( *XRCore::GetSession(), 0, &nNumOfSupportedFormats, nullptr );

			// Get swapchain formats supported by the runtime
			std::vector< int64_t > vRuntimeSwapChainFormats;
		vRuntimeSwapChainFormats.resize( nNumOfSupportedFormats );

		m_LastCallResult = xrEnumerateSwapchainFormats( *XRCore::GetSession(), nNumOfSupportedFormats,
			&nNumOfSupportedFormats, vRuntimeSwapChainFormats.data() );

		// Look for a matching texture format that the app requested vs what's supported by the runtime
		KM_CORE_INFO( "Runtime supports the following texture formats in order of preference:" );
		uint32_t nNum = 0;
		int64_t nMatchColor = 0;
		int64_t nMatchDepth = 0;

		for (int64_t SwapChainFormat : vRuntimeSwapChainFormats)
		{
			KM_CORE_INFO( "{}. {} ({})", ++nNum, XRGraphics::GetTextureFormatName(SwapChainFormat), SwapChainFormat);

			// Look for matching color texture format
			for (size_t i = 0; i < vAppTextureFormats.size(); i++)
			{
				if (nMatchColor == 0)
					nMatchColor = SwapChainFormat == vAppTextureFormats[i] ? SwapChainFormat : 0;
			}

			// Look for matching depth texture format
			if (m_bDepthHandling)
			{
				for (size_t i = 0; i < vAppDepthFormats.size(); i++)
				{
					// Check if the this texture format is a depth format
					if (nMatchDepth == 0 && XRGraphics::IsDepth(SwapChainFormat))
					{
						nMatchDepth = SwapChainFormat == vAppDepthFormats[i] ? SwapChainFormat : 0;
					}
				}
			}
		}

		// Choose the strongest runtime preference if app texture request can't be found
		if (nMatchColor == 0)
			nMatchColor = vRuntimeSwapChainFormats[0];

		i_TextureFormat = nMatchColor;
		KM_CORE_INFO( "XR Texture color format will be {} ({})", XRGraphics::GetTextureFormatName(nMatchColor), nMatchColor);

		// Process depth textures if available
		if (m_bDepthHandling && vAppDepthFormats.size() > 0)
		{
			// Choose a known depth format if app texture request can't be found or the app requested an invalid depth format
			if (nMatchDepth == 0 || !XRGraphics::IsDepth( nMatchDepth ))
				nMatchDepth = XRGraphics::GetDefaultDepthFormat();

			i_DepthFormat = nMatchDepth;
			KM_CORE_INFO( "XR Texture depth format will be {} ({})", XRGraphics::GetTextureFormatName( nMatchDepth ), nMatchDepth );
		}
		else
		{
			KM_CORE_INFO( "Session will not support depth textures" );
			m_bDepthHandling = false;
		}
	}

	bool XRRender::GetVisibilityMask( EXREye eEye, EMaskType eMaskType, std::vector<float>& vMaskVertices, std::vector<uint32_t>& vMaskIndices )
	{
		if (XRCore::GetInstance() == XR_NULL_HANDLE || XRCore::GetSession() == XR_NULL_HANDLE)
			return false;

		// Convert mask type to native OpenXR mask type
		XrVisibilityMaskTypeKHR xrVisibilityMaskType = XR_VISIBILITY_MASK_TYPE_HIDDEN_TRIANGLE_MESH_KHR;
		switch (eMaskType)
		{
		case EMaskType::MASK_VISIBLE:
			xrVisibilityMaskType = XR_VISIBILITY_MASK_TYPE_VISIBLE_TRIANGLE_MESH_KHR;
			break;
		case EMaskType::MASK_HIDDEN:
			xrVisibilityMaskType = XR_VISIBILITY_MASK_TYPE_HIDDEN_TRIANGLE_MESH_KHR;
			break;
		case EMaskType::MASK_LINE_LOOP:
			xrVisibilityMaskType = XR_VISIBILITY_MASK_TYPE_LINE_LOOP_KHR;
			break;
		}

		PFN_xrGetVisibilityMaskKHR xrGetVisibilityMaskKHR = nullptr;
		m_LastCallResult = xrGetInstanceProcAddr( *XRCore::GetInstance(), "xrGetVisibilityMaskKHR",
			(PFN_xrVoidFunction*)&xrGetVisibilityMaskKHR );

		if (m_LastCallResult != XR_SUCCESS)
			return false;

		// Get index and vertex counts
		XrVisibilityMaskKHR pXRVisibilityMask = { XR_TYPE_VISIBILITY_MASK_KHR };
		pXRVisibilityMask.indexCapacityInput = 0;
		pXRVisibilityMask.vertexCapacityInput = 0;

		m_LastCallResult = 
			xrGetVisibilityMaskKHR( *XRCore::GetSession(), XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, eEye == EYE_LEFT ? 0 : 1, xrVisibilityMaskType, &pXRVisibilityMask);

		if (m_LastCallResult != XR_SUCCESS)
			return false;

		if (pXRVisibilityMask.indexCountOutput == 0 && pXRVisibilityMask.vertexCountOutput == 0)
		{
			KM_CORE_WARN( "Runtime does not have a Visibility Mask for eye ({})", eEye );
			return false;
		}
		else if (xrVisibilityMaskType == XR_VISIBILITY_MASK_TYPE_LINE_LOOP_KHR && pXRVisibilityMask.indexCountOutput % 3 != 0)
		{
			KM_CORE_ERROR( "Runtime returned an invalid Visibility Mask" );
			return false;
		}

		// Setup target vectors for the receiving vertices and indices
		std::vector< XrVector2f > vXrVertices;
		uint32_t nVertexCount = pXRVisibilityMask.vertexCountOutput;
		uint32_t nIndexCount = pXRVisibilityMask.indexCountOutput;

		vXrVertices.reserve( nVertexCount );
		vMaskIndices.reserve( nIndexCount );

		// Get mask vertices and indices from the runtime
		pXRVisibilityMask = { XR_TYPE_VISIBILITY_MASK_KHR };
		pXRVisibilityMask.vertexCapacityInput = nVertexCount;
		pXRVisibilityMask.indexCapacityInput = nIndexCount;
		pXRVisibilityMask.indexCountOutput = 0;
		pXRVisibilityMask.vertexCountOutput = 0;
		pXRVisibilityMask.indices = vMaskIndices.data();
		pXRVisibilityMask.vertices = vXrVertices.data();

		m_LastCallResult = xrGetVisibilityMaskKHR( *XRCore::GetSession(), XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, eEye == EYE_LEFT ? 0 : 1, xrVisibilityMaskType, &pXRVisibilityMask );

		if (m_LastCallResult != XR_SUCCESS)
			return false;

		if (!pXRVisibilityMask.indices || !pXRVisibilityMask.vertices)
		{
			KM_CORE_WARN(
				"Runtime did not return any indices or vertices for eye ({}). Try again on  "
				"XrEventDataVisibilityMaskChangedKHR::XR_TYPE_EVENT_DATA_VISIBILITY_MASK_CHANGED_KHR",
				eEye );
			return false;
		}

		// Fill in return vertices
		bool b = vMaskVertices.empty();
		for (size_t i = 0; i < pXRVisibilityMask.vertexCountOutput; i++)
		{
			vMaskVertices.push_back( pXRVisibilityMask.vertices[i].x );
			vMaskVertices.push_back( pXRVisibilityMask.vertices[i].y );
		}

		vMaskIndices.insert( vMaskIndices.end(), &pXRVisibilityMask.indices[0], &pXRVisibilityMask.indices[nIndexCount] );
		return true;
	}

	void XRRender::GenerateSwapchains( bool bIsDepth )
	{
		for (size_t i = 0; i < m_Views.size(); i++)
		{
			XrSwapchain xrSwapChain;
			XrSwapchainCreateInfo xrSwapChainCreateInfo{ XR_TYPE_SWAPCHAIN_CREATE_INFO };
			xrSwapChainCreateInfo.arraySize = u_TextureArraySize;
			xrSwapChainCreateInfo.format = bIsDepth ? i_DepthFormat : i_TextureFormat;
			xrSwapChainCreateInfo.width = m_ViewConfigs[i].recommendedImageRectWidth;
			xrSwapChainCreateInfo.height = m_ViewConfigs[i].recommendedImageRectHeight;
			xrSwapChainCreateInfo.mipCount = u_TextureMipCount;
			xrSwapChainCreateInfo.faceCount = 1;
			xrSwapChainCreateInfo.sampleCount = m_ViewConfigs[i].recommendedSwapchainSampleCount;

			if (bIsDepth)
			{
				xrSwapChainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			}
			else
			{
				xrSwapChainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
			}

			m_LastCallResult = xrCreateSwapchain( *XRCore::GetSession(), &xrSwapChainCreateInfo, &xrSwapChain);

			if (bIsDepth)
			{
				m_SwapChainsDepth.push_back( xrSwapChain );
				KM_CORE_INFO(
					"Depth Swapchain created for eye ({}). Textures are {}x{} with {} sample(s) and {} Mip(s)",
					i,
					u_TextureWidth,
					u_TextureHeight,
					m_ViewConfigs[i].recommendedSwapchainSampleCount,
					u_TextureMipCount );
			}
			else
			{
				m_SwapChainsColor.push_back( xrSwapChain );
				KM_CORE_INFO(
					"Color Swapchain created for eye ({}). Textures are {}x{} with {} sample(s) and {} Mip(s)",
					i,
					u_TextureWidth,
					u_TextureHeight,
					m_ViewConfigs[i].recommendedSwapchainSampleCount,
					u_TextureMipCount );
			}
		}
	}

	int64_t XRRender::GetTextureFormat()
	{
		return i_TextureFormat;
	}

	uint32_t XRRender::GetTextureHeight()
	{
		return u_TextureHeight;
	}

	uint32_t XRRender::GetTextureMipCount()
	{
		return u_TextureMipCount;
	}

	uint32_t XRRender::GetTextureWidth()
	{
		return u_TextureWidth;
	}


}