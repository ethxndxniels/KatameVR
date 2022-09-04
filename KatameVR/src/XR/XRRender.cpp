#include "XRRender.h"

#include "XRInput.h"

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

	ID3D11DepthStencilView* XRRender::m_DSV = nullptr;
	ID3D11RenderTargetView* XRRender::m_RTV = nullptr;

	std::vector<XrSpace> XRRender::m_VisualizedSpaces = {};


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

	void XRRender::BeginFrame( unsigned int index )
	{
		// Set up where on the render target we want to draw, the view has a 
		//D3D11_VIEWPORT viewport = CD3D11_VIEWPORT( m_Views[index]., offset_y, extent_width, extent_height);
		//XRGraphics::RSSetViewports( 1, viewport );

		// Wipe our swapchain color and depth target clean, and then set them up for rendering!
		float clear[] = { 20.0f / 255.0f, 2.0f / 255.0f, 30.0f / 255.0f, 1 };
		XRGraphics::ClearRenderTargetView( m_RTV, clear );
		XRGraphics::ClearDepthStencilView( m_DSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );
		XRGraphics::OMSetRenderTargets( 1, m_RTV, m_DSV );
	}

	void XRRender::RenderFrame()
	{
		XrFrameWaitInfo frameWaitInfo{ XR_TYPE_FRAME_WAIT_INFO };
		XrFrameState frameState{ XR_TYPE_FRAME_STATE };
		xrWaitFrame( *XRCore::GetSession(), &frameWaitInfo, &frameState);

		XrFrameBeginInfo frameBeginInfo{ XR_TYPE_FRAME_BEGIN_INFO };
		xrBeginFrame( *XRCore::GetSession(), &frameBeginInfo );

		std::vector<XrCompositionLayerBaseHeader*> layers;
		XrCompositionLayerProjection layer{ XR_TYPE_COMPOSITION_LAYER_PROJECTION };
		std::vector<XrCompositionLayerProjectionView> projectionLayerViews;
		if (frameState.shouldRender == XR_TRUE) {
			if (RenderLayer( frameState.predictedDisplayTime, projectionLayerViews, layer )) {
				layers.push_back( reinterpret_cast<XrCompositionLayerBaseHeader*>(&layer) );
			}
		}

		XrFrameEndInfo frameEndInfo{ XR_TYPE_FRAME_END_INFO };
		frameEndInfo.displayTime = frameState.predictedDisplayTime;
		frameEndInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND;
		frameEndInfo.layerCount = (uint32_t)layers.size();
		frameEndInfo.layers = layers.data();
		xrEndFrame( *XRCore::GetSession(), &frameEndInfo );
	}

	struct Cube {
		XrPosef Pose;
		XrVector3f Scale;
	};
	bool XRRender::RenderLayer( XrTime predictedDisplayTime, std::vector<XrCompositionLayerProjectionView>& projectionLayerViews, XrCompositionLayerProjection& layer )
	{
		XrViewState viewState{ XR_TYPE_VIEW_STATE };
		uint32_t viewCapacityInput = (uint32_t)m_Views.size();
		uint32_t viewCountOutput;

		XrViewLocateInfo viewLocateInfo{ XR_TYPE_VIEW_LOCATE_INFO };
		viewLocateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
		viewLocateInfo.displayTime = predictedDisplayTime;
		viewLocateInfo.space = *XRCore::GetSpace();

		m_LastCallResult = xrLocateViews( *XRCore::GetSession(), &viewLocateInfo, &viewState, viewCapacityInput, &viewCountOutput, m_views.data() );
		if ((viewState.viewStateFlags & XR_VIEW_STATE_POSITION_VALID_BIT) == 0 ||
			(viewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT) == 0) {
			return false;  // There is no valid tracking poses for the views.
		}

		viewCountOutput == viewCapacityInput;
		viewCountOutput == m_ViewConfigs.size();
		viewCountOutput == m_SwapChainsColor.size();

		projectionLayerViews.resize( viewCountOutput );

		// For each locatable space that we want to visualize, render a 25cm cube.
		std::vector<Cube> cubes;

		for (XrSpace visualizedSpace : m_VisualizedSpaces) {
			XrSpaceLocation spaceLocation{ XR_TYPE_SPACE_LOCATION };
			m_LastCallResult = xrLocateSpace( visualizedSpace, *XRCore::GetSpace(), predictedDisplayTime, &spaceLocation);
			if (m_LastCallResult == XR_SUCCESS) {
				if ((spaceLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0 &&
					(spaceLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0) {
					cubes.push_back( Cube{ spaceLocation.pose, {0.25f, 0.25f, 0.25f} } );
				}
			}
			else {
				KM_CORE_WARN( "Unable to locate a visualized reference space in app space: {}", m_LastCallResult );
			}
		}

		// Render a 10cm cube scaled by grabAction for each hand. Note renderHand will only be
		// true when the application has focus.
		for (auto hand : { Side::LEFT, Side::RIGHT }) {
			XrSpaceLocation spaceLocation{ XR_TYPE_SPACE_LOCATION };
			m_LastCallResult = xrLocateSpace( XRInput::.handSpace[hand], m_appSpace, predictedDisplayTime, &spaceLocation );
			if (m_LastCallResult == XR_SUCCESS) {
				if ((spaceLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0 &&
					(spaceLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0) {
					float scale = 0.1f * m_input.handScale[hand];
					cubes.push_back( Cube{ spaceLocation.pose, {scale, scale, scale} } );
				}
			}
			else {
				// Tracking loss is expected when the hand is not active so only log a message
				// if the hand is active.
				if (m_input.handActive[hand] == XR_TRUE) {
					const char* handName[] = { "left", "right" };
					Log::Write( Log::Level::Verbose,
						Fmt( "Unable to locate %s hand action space in app space: %d", handName[hand], res ) );
				}
			}
		}

		// Render view to the appropriate part of the swapchain image.
		for (uint32_t i = 0; i < viewCountOutput; i++) {
			// Each view has a separate swapchain which is acquired, rendered to, and released.
			const Swapchain viewSwapchain = m_swapchains[i];

			XrSwapchainImageAcquireInfo acquireInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };

			uint32_t swapchainImageIndex;
			CHECK_XRCMD( xrAcquireSwapchainImage( viewSwapchain.handle, &acquireInfo, &swapchainImageIndex ) );

			XrSwapchainImageWaitInfo waitInfo{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
			waitInfo.timeout = XR_INFINITE_DURATION;
			CHECK_XRCMD( xrWaitSwapchainImage( viewSwapchain.handle, &waitInfo ) );

			projectionLayerViews[i] = { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW };
			projectionLayerViews[i].pose = m_views[i].pose;
			projectionLayerViews[i].fov = m_views[i].fov;
			projectionLayerViews[i].subImage.swapchain = viewSwapchain.handle;
			projectionLayerViews[i].subImage.imageRect.offset = { 0, 0 };
			projectionLayerViews[i].subImage.imageRect.extent = { viewSwapchain.width, viewSwapchain.height };

			const XrSwapchainImageBaseHeader* const swapchainImage = m_swapchainImages[viewSwapchain.handle][swapchainImageIndex];
			m_graphicsPlugin->RenderView( projectionLayerViews[i], swapchainImage, m_colorSwapchainFormat, cubes );

			XrSwapchainImageReleaseInfo releaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
			CHECK_XRCMD( xrReleaseSwapchainImage( viewSwapchain.handle, &releaseInfo ) );
		}

		layer.space = m_appSpace;
		layer.layerFlags =
			m_options->Parsed.EnvironmentBlendMode == XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND
			? XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT | XR_COMPOSITION_LAYER_UNPREMULTIPLIED_ALPHA_BIT
			: 0;
		layer.viewCount = (uint32_t)projectionLayerViews.size();
		layer.views = projectionLayerViews.data();
		return true;
	}

	DirectX::XMMATRIX XRRender::GetView( unsigned int index )
	{
		return  XMMatrixInverse( nullptr, XMMatrixAffineTransformation(
			DirectX::g_XMOne, DirectX::g_XMZero,
			DirectX::XMLoadFloat4( (DirectX::XMFLOAT4*)&m_Views[index].pose.orientation ),
			DirectX::XMLoadFloat3( (DirectX::XMFLOAT3*)&m_Views[index].pose.position ) ) );
	}

	DirectX::XMMATRIX XRRender::GetProjection( unsigned int index )
	{
		if ( m_Views[index].fov.angleLeft == m_Views[index].fov.angleRight )
			return {};

		const float left = 0.05f * tanf( m_Views[index].fov.angleLeft );
		const float right = 0.05f * tanf( m_Views[index].fov.angleRight );
		const float down = 0.05f * tanf( m_Views[index].fov.angleDown );
		const float up = 0.05f * tanf( m_Views[index].fov.angleUp );

		return DirectX::XMMatrixPerspectiveOffCenterRH( left, right, down, up, 0.05f, 200.0f );
	}

	std::vector<XrView> XRRender::GetViews()
	{
		return m_Views;
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