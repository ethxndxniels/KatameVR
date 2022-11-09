#include "XRCore.h"
#include "../Core/Log.h"

#include "../Core/Application.h"
#include "../Graphics/Common.h"

namespace Katame
{
    namespace
    {

#if !defined(XR_USE_PLATFORM_WIN32)
#define strcpy_s(dest, source) strncpy((dest), (source), sizeof(dest))
#endif

        namespace Math
        {
            namespace Pose
            {
                XrPosef Identity()
                {
                    XrPosef t{};
                    t.orientation.w = 1;
                    return t;
                }

                XrPosef Translation( const XrVector3f& translation )
                {
                    XrPosef t = Identity();
                    t.position = translation;
                    return t;
                }

                XrPosef RotateCCWAboutYAxis( float radians, XrVector3f translation )
                {
                    XrPosef t = Identity();
                    t.orientation.x = 0.f;
                    t.orientation.y = std::sin( radians * 0.5f );
                    t.orientation.z = 0.f;
                    t.orientation.w = std::cos( radians * 0.5f );
                    t.position = translation;
                    return t;
                }
            }
        }
    }

    inline XrReferenceSpaceCreateInfo GetXrReferenceSpaceCreateInfo( const std::string& referenceSpaceTypeStr )
    {
        XrReferenceSpaceCreateInfo referenceSpaceCreateInfo{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
        referenceSpaceCreateInfo.poseInReferenceSpace = Math::Pose::Identity();
        if (EqualsIgnoreCase( referenceSpaceTypeStr, "View" ))
        {
            referenceSpaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
        }
        else if (EqualsIgnoreCase( referenceSpaceTypeStr, "ViewFront" )) 
        {
            // Render head-locked 2m in front of device.
            referenceSpaceCreateInfo.poseInReferenceSpace = Math::Pose::Translation( { 0.f, 0.f, -2.f } ),
                referenceSpaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
        }
        else if (EqualsIgnoreCase( referenceSpaceTypeStr, "Local" )) 
        {
            referenceSpaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
        }
        else if (EqualsIgnoreCase( referenceSpaceTypeStr, "Stage" )) 
        {
            referenceSpaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE;
        }
        else if (EqualsIgnoreCase( referenceSpaceTypeStr, "StageLeft" )) 
        {
            referenceSpaceCreateInfo.poseInReferenceSpace = Math::Pose::RotateCCWAboutYAxis( 0.f, { -2.f, 0.f, -2.f } );
            referenceSpaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE;
        }
        else if (EqualsIgnoreCase( referenceSpaceTypeStr, "StageRight" )) 
        {
            referenceSpaceCreateInfo.poseInReferenceSpace = Math::Pose::RotateCCWAboutYAxis( 0.f, { 2.f, 0.f, -2.f } );
            referenceSpaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE;
        }
        else if (EqualsIgnoreCase( referenceSpaceTypeStr, "StageLeftRotated" )) 
        {
            referenceSpaceCreateInfo.poseInReferenceSpace = Math::Pose::RotateCCWAboutYAxis( 3.14f / 3.f, { -2.f, 0.5f, -2.f } );
            referenceSpaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE;
        }
        else if (EqualsIgnoreCase( referenceSpaceTypeStr, "StageRightRotated" )) 
        {
            referenceSpaceCreateInfo.poseInReferenceSpace = Math::Pose::RotateCCWAboutYAxis( -3.14f / 3.f, { 2.f, 0.5f, -2.f } );
            referenceSpaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE;
        }
        else 
        {
            //KM_CORE_ERROR( "Unknown reference space type {}", referenceSpaceTypeStr.c_str() );
            throw std::exception{};
        }
        return referenceSpaceCreateInfo;
    }

    XRCore::XRCore( Graphics* gfx )
        : gfx( gfx )
    {
    }

    XRCore::~XRCore()
    {
    }

    void XRCore::CreateInstance()
    {
        LogLayersAndExtensions();

        CreateInstanceInternal();

        LogInstanceInfo();
    }

    void XRCore::InitializeSystem()
    {
        XrSystemGetInfo systemInfo{ XR_TYPE_SYSTEM_GET_INFO };
        systemInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
        m_LastCallResult = xrGetSystem( m_Instance, &systemInfo, &m_SystemId );
        if (m_LastCallResult != XR_SUCCESS)
            KM_CORE_ERROR( "Failed to get system." );
    }

    void XRCore::InitializeDevice()
    {
        LogViewConfigurations();

        // The graphics API can initialize the graphics device now that the systemId and instance
        // handle are available.
        gfx->InitializeDevice( m_Instance, m_SystemId );
    }

    void XRCore::InitializeSession() 
    {
        {
            KM_CORE_INFO( "Creating session..." );

            XrGraphicsBindingD3D11KHR binding = { XR_TYPE_GRAPHICS_BINDING_D3D11_KHR };
            binding.device = gfx->m_Device;
            XrSessionCreateInfo createInfo{ XR_TYPE_SESSION_CREATE_INFO };
            createInfo.next = &binding;
            createInfo.systemId = m_SystemId;
            xrCreateSession( m_Instance, &createInfo, &m_Session );
        }

        LogReferenceSpaces();
        InitializeActions();
        CreateVisualizedSpaces();

        {
            XrReferenceSpaceCreateInfo referenceSpaceCreateInfo = GetXrReferenceSpaceCreateInfo( "Local" );
            xrCreateReferenceSpace( m_Session, &referenceSpaceCreateInfo, &m_Space );
        }
    }

    void XRCore::CreateSwapchains()
    {
        // Read graphics properties for preferred swapchain length and logging.
        XrSystemProperties systemProperties{ XR_TYPE_SYSTEM_PROPERTIES };
        xrGetSystemProperties( m_Instance, m_SystemId, &systemProperties );

        // Log system properties.
        KM_CORE_INFO( "System Properties: Name={} VendorId={}", systemProperties.systemName, systemProperties.vendorId );
        KM_CORE_TRACE( "System Graphics Properties: MaxWidth={} MaxHeight={} MaxLayers={}",
            systemProperties.graphicsProperties.maxSwapchainImageWidth,
            systemProperties.graphicsProperties.maxSwapchainImageHeight,
            systemProperties.graphicsProperties.maxLayerCount );
        KM_CORE_TRACE( "System Tracking Properties: OrientationTracking={} PositionTracking={}",
            systemProperties.trackingProperties.orientationTracking == XR_TRUE ? "True" : "False",
            systemProperties.trackingProperties.positionTracking == XR_TRUE ? "True" : "False" );

        // Note: No other view configurations exist at the time this code was written. If this
        // condition is not met, the project will need to be audited to see how support should be
        // added.
        //CHECK_MSG( m_options->Parsed.ViewConfigType == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
        //    "Unsupported view configuration type" );

        // Query and cache view configuration views.
        uint32_t viewCount;
        xrEnumerateViewConfigurationViews( m_Instance, m_SystemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &viewCount, nullptr );
        m_configViews.resize( viewCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW } );
        xrEnumerateViewConfigurationViews( m_Instance, m_SystemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, viewCount,
            &viewCount, m_configViews.data() );

        // Create and cache view buffer for xrLocateViews later.
        m_views.resize( viewCount, { XR_TYPE_VIEW } );

        // Create the swapchain and get the images.
        if (viewCount > 0) 
        {
            // Select a swapchain format.
            uint32_t swapchainFormatCount;
            xrEnumerateSwapchainFormats( m_Session, 0, &swapchainFormatCount, nullptr );
            std::vector<int64_t> swapchainFormats( swapchainFormatCount );
            xrEnumerateSwapchainFormats( m_Session, (uint32_t)swapchainFormats.size(), &swapchainFormatCount, swapchainFormats.data() );
            m_colorSwapchainFormat = gfx->SelectColorSwapchainFormat( swapchainFormats );

            // Print swapchain formats and the selected one.
            {
                std::string swapchainFormatsString;
                for (int64_t format : swapchainFormats) 
                {
                    const bool selected = format == m_colorSwapchainFormat;
                    swapchainFormatsString += " ";
                    if (selected) {
                        swapchainFormatsString += "[";
                    }
                    swapchainFormatsString += std::to_string( format );
                    if (selected) {
                        swapchainFormatsString += "]";
                    }
                }
                KM_CORE_TRACE( "Swapchain Formats: {}", swapchainFormatsString.c_str() );
            }

            // Create a swapchain for each view.
            for (uint32_t i = 0; i < viewCount; i++) 
            {
                const XrViewConfigurationView& vp = m_configViews[i];
                KM_CORE_TRACE( "Creating swapchain for view {} with dimensions Width={} Height={} SampleCount={}", i,
                        vp.recommendedImageRectWidth, vp.recommendedImageRectHeight, vp.recommendedSwapchainSampleCount );

                // Create the swapchain.
                XrSwapchainCreateInfo swapchainCreateInfo{ XR_TYPE_SWAPCHAIN_CREATE_INFO };
                swapchainCreateInfo.arraySize = 1;
                swapchainCreateInfo.format = m_colorSwapchainFormat;
                swapchainCreateInfo.width = vp.recommendedImageRectWidth;
                swapchainCreateInfo.height = vp.recommendedImageRectHeight;
                swapchainCreateInfo.mipCount = 1;
                swapchainCreateInfo.faceCount = 1;
                swapchainCreateInfo.sampleCount = 1;
                swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
                Swapchain swapchain;
                swapchain.width = swapchainCreateInfo.width;
                swapchain.height = swapchainCreateInfo.height;
                xrCreateSwapchain( m_Session, &swapchainCreateInfo, &swapchain.handle );

                m_swapchains.push_back( swapchain );

                uint32_t imageCount;
                xrEnumerateSwapchainImages( swapchain.handle, 0, &imageCount, nullptr );
                // XXX This should really just return XrSwapchainImageBaseHeader*
                std::vector<XrSwapchainImageBaseHeader*> swapchainImages =
                    gfx->AllocateSwapchainImageStructs( imageCount, swapchainCreateInfo );
                xrEnumerateSwapchainImages( swapchain.handle, imageCount, &imageCount, swapchainImages[0] );

                m_swapchainImages.insert( std::make_pair( swapchain.handle, std::move( swapchainImages ) ) );
            }
        }
    }

    bool XRCore::IsSessionRunning()
    {
        return m_sessionRunning;
    }

    std::pair<XrPosef,XrVector3f> XRCore::GetLeftHand()
    {
        XrSpaceLocation spaceLocation{ XR_TYPE_SPACE_LOCATION };
        m_LastCallResult = xrLocateSpace( m_Input.handSpace[Side::LEFT], m_Space, m_PredictedDisplayTime, &spaceLocation );
        if (XR_UNQUALIFIED_SUCCESS( m_LastCallResult ))
        {
            if ((spaceLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0 &&
                (spaceLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0) 
            {
                float scale = 0.005f * m_Input.handScale[Side::LEFT];
                XrPosef thepose = spaceLocation.pose;
                thepose.position.x += m_Pose.position.x;
                thepose.position.y += m_Pose.position.y;
                thepose.position.z += m_Pose.position.z;
                return { thepose, {scale, scale, scale} };
            }
        }
        else 
        {
            // Tracking loss is expected when the hand is not active so only log a message
            // if the hand is active.
            if (m_Input.handActive[Side::LEFT] == XR_TRUE)
            {
                const char* handName[] = { "left", "right" };
                KM_CORE_INFO( "Unable to locate %s hand action space in app space: %d", handName[Side::LEFT], m_LastCallResult );
            }
        }
        return {};
    }

    std::pair<XrPosef, XrVector3f> XRCore::GetRightHand()
    {
        XrSpaceLocation spaceLocation{ XR_TYPE_SPACE_LOCATION };
        m_LastCallResult = xrLocateSpace( m_Input.handSpace[Side::RIGHT], m_Space, m_PredictedDisplayTime, &spaceLocation );
        if (XR_UNQUALIFIED_SUCCESS( m_LastCallResult ))
        {
            if ((spaceLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0 &&
                (spaceLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0)
            {
                float scale = 0.005f * m_Input.handScale[Side::RIGHT];
                XrPosef thepose = spaceLocation.pose;
                thepose.position.x += m_Pose.position.x;
                thepose.position.y += m_Pose.position.y;
                thepose.position.z += m_Pose.position.z;
                return { thepose, {scale, scale, scale} };
            }
        }
        else
        {
            // Tracking loss is expected when the hand is not active so only log a message
            // if the hand is active.
            if (m_Input.handActive[Side::RIGHT] == XR_TRUE)
            {
                const char* handName[] = { "left", "right" };
                KM_CORE_INFO( "Unable to locate %s hand action space in app space: %d", handName[Side::RIGHT], m_LastCallResult );
            }
        }
        return {};
    }

    void XRCore::PollEvents( bool* exitRenderLoop, bool* requestRestart )
    {
        *exitRenderLoop = *requestRestart = false;

        // Process all pending messages.
        while (const XrEventDataBaseHeader* event = TryReadNextEvent()) 
        {
            switch (event->type) 
            {
            case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
            {
                const auto& instanceLossPending = *reinterpret_cast<const XrEventDataInstanceLossPending*>(event);
                KM_CORE_TRACE( "XrEventDataInstanceLossPending by {}", instanceLossPending.lossTime );
                *exitRenderLoop = true;
                *requestRestart = true;
                return;
            }
            case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
            {
                auto sessionStateChangedEvent = *reinterpret_cast<const XrEventDataSessionStateChanged*>(event);
                HandleSessionStateChangedEvent( sessionStateChangedEvent, exitRenderLoop, requestRestart );
                break;
            }
            case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
                LogActionSourceName( m_Input.grabAction, "Grab" );
                LogActionSourceName( m_Input.quitAction, "Quit" );
                LogActionSourceName( m_Input.poseAction, "Pose" );
                LogActionSourceName( m_Input.vibrateAction, "Vibrate" );
                LogActionSourceName( m_Input.leftThumbstickXAction, "Left Thumbstick X" );
                LogActionSourceName( m_Input.leftThumbstickYAction, "Left Thumbstick Y" );
                LogActionSourceName( m_Input.rightThumbstickXAction, "Right Thumbstick X" );
                LogActionSourceName( m_Input.rightThumbstickYAction, "Right Thumbstick Y" );
                break;
            case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING:
            default: 
            {
                KM_CORE_TRACE( "Ignoring event type {}", event->type );
                break;
            }
            }
        }
    }

    void XRCore::PollActions()
    {
        m_Input.handActive = { XR_FALSE, XR_FALSE };

        // Sync actions
        const XrActiveActionSet activeActionSet{ m_Input.actionSet, XR_NULL_PATH };
        XrActionsSyncInfo syncInfo{ XR_TYPE_ACTIONS_SYNC_INFO };
        syncInfo.countActiveActionSets = 1;
        syncInfo.activeActionSets = &activeActionSet;
        xrSyncActions( m_Session, &syncInfo );

        // Get pose and grab action state and start haptic vibrate when hand is 90% squeezed.
        for (auto hand : { Side::LEFT, Side::RIGHT })
        {
            XrActionStateGetInfo getInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
            getInfo.action = m_Input.grabAction;
            getInfo.subactionPath = m_Input.handSubactionPath[hand];

            XrActionStateFloat grabValue{ XR_TYPE_ACTION_STATE_FLOAT };
            xrGetActionStateFloat( m_Session, &getInfo, &grabValue );
            if (grabValue.isActive == XR_TRUE)
            {
                // Scale the rendered hand by 1.0f (open) to 0.5f (fully squeezed).
                m_Input.handScale[hand] = 1.0f - 0.5f * grabValue.currentState;
                if (grabValue.currentState > 0.9f) 
                {
                    XrHapticVibration vibration{ XR_TYPE_HAPTIC_VIBRATION };
                    vibration.amplitude = 0.5;
                    vibration.duration = XR_MIN_HAPTIC_DURATION;
                    vibration.frequency = XR_FREQUENCY_UNSPECIFIED;

                    XrHapticActionInfo hapticActionInfo{ XR_TYPE_HAPTIC_ACTION_INFO };
                    hapticActionInfo.action = m_Input.vibrateAction;
                    hapticActionInfo.subactionPath = m_Input.handSubactionPath[hand];
                    xrApplyHapticFeedback( m_Session, &hapticActionInfo, (XrHapticBaseHeader*)&vibration );
                }
            }

            getInfo.action = m_Input.poseAction;
            XrActionStatePose poseState{ XR_TYPE_ACTION_STATE_POSE };
            xrGetActionStatePose( m_Session, &getInfo, &poseState );
            m_Input.handActive[hand] = poseState.isActive;
        }

        // Left Controller X
        XrActionStateGetInfo getInfoLeftThumbstickX{ XR_TYPE_ACTION_STATE_GET_INFO };
        getInfoLeftThumbstickX.action = m_Input.leftThumbstickXAction;
        getInfoLeftThumbstickX.subactionPath = m_Input.handSubactionPath[Side::LEFT];

        XrActionStateFloat leftThumbstickXValue{ XR_TYPE_ACTION_STATE_FLOAT };
        xrGetActionStateFloat( m_Session, &getInfoLeftThumbstickX, &leftThumbstickXValue );
        if (leftThumbstickXValue.changedSinceLastSync == XR_TRUE)
            m_Pose.position.x += leftThumbstickXValue.currentState * 0.1f;

        // Left Controller Y
        XrActionStateGetInfo getInfoLeftThumbstickY{ XR_TYPE_ACTION_STATE_GET_INFO };
        getInfoLeftThumbstickY.action = m_Input.leftThumbstickYAction;
        getInfoLeftThumbstickY.subactionPath = m_Input.handSubactionPath[Side::LEFT];

        XrActionStateFloat leftThumbstickYValue{ XR_TYPE_ACTION_STATE_FLOAT };
        xrGetActionStateFloat( m_Session, &getInfoLeftThumbstickY, &leftThumbstickYValue );
        if (leftThumbstickYValue.changedSinceLastSync == XR_TRUE)
            m_Pose.position.z += -1.0f * leftThumbstickYValue.currentState * 0.1f;

        // Right Controller X
        XrActionStateGetInfo getInfoRightThumbstickX{ XR_TYPE_ACTION_STATE_GET_INFO };
        getInfoRightThumbstickX.action = m_Input.rightThumbstickXAction;
        getInfoRightThumbstickX.subactionPath = m_Input.handSubactionPath[Side::RIGHT];

        XrActionStateFloat rightThumbstickXValue{ XR_TYPE_ACTION_STATE_FLOAT };
        xrGetActionStateFloat( m_Session, &getInfoRightThumbstickX, &rightThumbstickXValue );
        if (rightThumbstickXValue.changedSinceLastSync == XR_TRUE)
            m_Pose.position.x += rightThumbstickXValue.currentState;

        // Right Controller Y
        XrActionStateGetInfo getInfoRightThumbstickY{ XR_TYPE_ACTION_STATE_GET_INFO };
        getInfoRightThumbstickY.action = m_Input.rightThumbstickYAction;
        getInfoRightThumbstickY.subactionPath = m_Input.handSubactionPath[Side::RIGHT];

        XrActionStateFloat rightThumbstickYValue{ XR_TYPE_ACTION_STATE_FLOAT };
        xrGetActionStateFloat( m_Session, &getInfoRightThumbstickY, &rightThumbstickYValue );
        if (rightThumbstickYValue.changedSinceLastSync == XR_TRUE)
            m_Pose.position.z +=  rightThumbstickYValue.currentState;

        // There were no subaction paths specified for the quit action, because we don't care which hand did it.
        XrActionStateGetInfo getInfo{ XR_TYPE_ACTION_STATE_GET_INFO, nullptr, m_Input.quitAction, XR_NULL_PATH };
        XrActionStateBoolean quitValue{ XR_TYPE_ACTION_STATE_BOOLEAN };
        xrGetActionStateBoolean( m_Session, &getInfo, &quitValue );
        if ((quitValue.isActive == XR_TRUE) && (quitValue.changedSinceLastSync == XR_TRUE) && (quitValue.currentState == XR_TRUE)) 
        {
            xrRequestExitSession( m_Session );
        }
    }

    void XRCore::RenderFrame()
    {
        XrFrameWaitInfo frameWaitInfo{ XR_TYPE_FRAME_WAIT_INFO };
        XrFrameState frameState{ XR_TYPE_FRAME_STATE };
        xrWaitFrame( m_Session, &frameWaitInfo, &frameState );

        XrFrameBeginInfo frameBeginInfo{ XR_TYPE_FRAME_BEGIN_INFO };
        xrBeginFrame( m_Session, &frameBeginInfo );

        std::vector<XrCompositionLayerBaseHeader*> layers;
        XrCompositionLayerProjection layer{ XR_TYPE_COMPOSITION_LAYER_PROJECTION };
        std::vector<XrCompositionLayerProjectionView> projectionLayerViews;
        if (frameState.shouldRender == XR_TRUE)
        {
            if (RenderLayer( frameState.predictedDisplayTime, projectionLayerViews, layer ))
            {
                layers.push_back( reinterpret_cast<XrCompositionLayerBaseHeader*>(&layer) );
            }
        }

        XrFrameEndInfo frameEndInfo{ XR_TYPE_FRAME_END_INFO };
        frameEndInfo.displayTime = frameState.predictedDisplayTime;
        frameEndInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
        frameEndInfo.layerCount = (uint32_t)layers.size();
        frameEndInfo.layers = layers.data();
        xrEndFrame( m_Session, &frameEndInfo );
    }

    void XRCore::LogLayersAndExtensions()
    {
        // Write out extension properties for a given layer.
        const auto logExtensions = []( const char* layerName, int indent = 0 ) 
        {
            uint32_t instanceExtensionCount;
            xrEnumerateInstanceExtensionProperties( layerName, 0, &instanceExtensionCount, nullptr );

            std::vector<XrExtensionProperties> extensions( instanceExtensionCount );
            for (XrExtensionProperties& extension : extensions) 
            {
                extension.type = XR_TYPE_EXTENSION_PROPERTIES;
            }

            xrEnumerateInstanceExtensionProperties( layerName, (uint32_t)extensions.size(), &instanceExtensionCount, extensions.data() );

            KM_CORE_INFO( "Available Extensions: {}", instanceExtensionCount );
            for (const XrExtensionProperties& extension : extensions)
            {
                KM_CORE_TRACE( "Name={} SpecVersion={}", extension.extensionName, extension.extensionVersion );
            }
        };

        // Log non-layer extensions (layerName==nullptr).
        logExtensions( nullptr );

        // Log layers and any of their extensions.
        {
            uint32_t layerCount;
            xrEnumerateApiLayerProperties( 0, &layerCount, nullptr );

            std::vector<XrApiLayerProperties> layers( layerCount );
            for (XrApiLayerProperties& layer : layers) {
                layer.type = XR_TYPE_API_LAYER_PROPERTIES;
            }

            xrEnumerateApiLayerProperties( (uint32_t)layers.size(), &layerCount, layers.data() );

            KM_CORE_INFO( "Available Layers: {}", layerCount );
            for (const XrApiLayerProperties& layer : layers) {
                KM_CORE_TRACE( "Name={} SpecVersion={} LayerVersion={} Description={}", layer.layerName, layer.specVersion,
                    layer.layerVersion, layer.description );
                logExtensions( layer.layerName, 4 );
            }
        }
    }

    void XRCore::CreateInstanceInternal()
    {
        // Create union of extensions required by platform and graphics plugins.
        std::vector<const char*> extensions;

        // Transform platform and graphics extension std::strings to C strings.
        const std::vector<std::string> platformExtensions = {};
        std::transform( platformExtensions.begin(), platformExtensions.end(), std::back_inserter( extensions ),
            []( const std::string& ext ) { return ext.c_str(); } );
        const std::vector<std::string> graphicsExtensions = { XR_KHR_D3D11_ENABLE_EXTENSION_NAME };
        std::transform( graphicsExtensions.begin(), graphicsExtensions.end(), std::back_inserter( extensions ),
            []( const std::string& ext ) { return ext.c_str(); } );

        XrInstanceCreateInfo createInfo{ XR_TYPE_INSTANCE_CREATE_INFO };
        createInfo.next = nullptr;
        createInfo.enabledExtensionCount = (uint32_t)extensions.size();
        createInfo.enabledExtensionNames = extensions.data();

        strcpy_s( createInfo.applicationInfo.applicationName, sizeof( "HelloXR" ) + 1, "HelloXR");
        createInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;

        m_LastCallResult = xrCreateInstance( &createInfo, &m_Instance );
        if (m_LastCallResult != XR_SUCCESS)
            KM_CORE_ERROR( "Failed to create instance." );
    }

    void XRCore::LogInstanceInfo()
    {
        XrInstanceProperties instanceProperties{ XR_TYPE_INSTANCE_PROPERTIES };
        xrGetInstanceProperties( m_Instance, &instanceProperties );

        KM_CORE_INFO( "Instance RuntimeName={} RuntimeVersion={}", instanceProperties.runtimeName, instanceProperties.runtimeVersion );
    }

    void XRCore::LogViewConfigurations()
    {

        uint32_t viewConfigTypeCount;
        xrEnumerateViewConfigurations( m_Instance, m_SystemId, 0, &viewConfigTypeCount, nullptr );

        std::vector<XrViewConfigurationType> viewConfigTypes( viewConfigTypeCount );
        xrEnumerateViewConfigurations( m_Instance, m_SystemId, viewConfigTypeCount, &viewConfigTypeCount, viewConfigTypes.data() );

        KM_CORE_INFO( "Available View Configuration Types: {}", viewConfigTypeCount );
        for (XrViewConfigurationType viewConfigType : viewConfigTypes)
        {
            KM_CORE_TRACE( "  View Configuration Type: {} {}", std::to_string( viewConfigType ), viewConfigType == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO ? "(Selected)" : "" );

            XrViewConfigurationProperties viewConfigProperties{ XR_TYPE_VIEW_CONFIGURATION_PROPERTIES };
            xrGetViewConfigurationProperties( m_Instance, m_SystemId, viewConfigType, &viewConfigProperties );

            KM_CORE_TRACE( "  View configuration FovMutable={}", viewConfigProperties.fovMutable == XR_TRUE ? "True" : "False" );

            uint32_t viewCount;
            xrEnumerateViewConfigurationViews( m_Instance, m_SystemId, viewConfigType, 0, &viewCount, nullptr );
            if (viewCount > 0)
            {
                std::vector<XrViewConfigurationView> views( viewCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW } );
                xrEnumerateViewConfigurationViews( m_Instance, m_SystemId, viewConfigType, viewCount, &viewCount, views.data() );

                for (uint32_t i = 0; i < views.size(); i++) {
                    const XrViewConfigurationView& view = views[i];

                    KM_CORE_TRACE( "    View [{}]: Recommended Width={} Height={} SampleCount={}", i,
                        view.recommendedImageRectWidth, view.recommendedImageRectHeight,
                        view.recommendedSwapchainSampleCount );
                    KM_CORE_TRACE( "    View [{}]:     Maximum Width={} Height={} SampleCount={}", i, view.maxImageRectWidth,
                        view.maxImageRectHeight, view.maxSwapchainSampleCount );
                }
            }
            else
            {
                KM_CORE_ERROR( "Empty view configuration type" );
            }

            LogEnvironmentBlendMode( viewConfigType );
        }
    }

    void XRCore::LogEnvironmentBlendMode( XrViewConfigurationType type )
    {
        uint32_t count;
        xrEnumerateEnvironmentBlendModes( m_Instance, m_SystemId, type, 0, &count, nullptr );

        KM_CORE_INFO( "Available Environment Blend Mode count : {}", count );

        std::vector<XrEnvironmentBlendMode> blendModes( count );
        xrEnumerateEnvironmentBlendModes( m_Instance, m_SystemId, type, count, &count, blendModes.data() );

        bool blendModeFound = false;
        for (XrEnvironmentBlendMode mode : blendModes)
        {
            const bool blendModeMatch = (mode == XR_ENVIRONMENT_BLEND_MODE_OPAQUE);
            KM_CORE_TRACE( "Environment Blend Mode {} : {}", std::to_string( mode ), blendModeMatch ? "(Selected)" : "" );
            blendModeFound |= blendModeMatch;
        }
    }
    void XRCore::LogReferenceSpaces()
    {
        uint32_t spaceCount;
        xrEnumerateReferenceSpaces( m_Session, 0, &spaceCount, nullptr );
        std::vector<XrReferenceSpaceType> spaces( spaceCount );
        xrEnumerateReferenceSpaces( m_Session, spaceCount, &spaceCount, spaces.data() );

        KM_CORE_INFO( "Available reference spaces: {}", spaceCount );
        for (XrReferenceSpaceType space : spaces) 
        {
            KM_CORE_TRACE( "  Name: {}", std::to_string(space) );
        }
    }
    void XRCore::InitializeActions()
    {
        // Create an action set.
        {
            XrActionSetCreateInfo actionSetInfo{ XR_TYPE_ACTION_SET_CREATE_INFO };
            strcpy_s( actionSetInfo.actionSetName, "gameplay" );
            strcpy_s( actionSetInfo.localizedActionSetName, "Gameplay" );
            actionSetInfo.priority = 0;
            xrCreateActionSet( m_Instance, &actionSetInfo, &m_Input.actionSet );
        }

        // Get the XrPath for the left and right hands - we will use them as subaction paths.
        xrStringToPath( m_Instance, "/user/hand/left", &m_Input.handSubactionPath[Side::LEFT] );
        xrStringToPath( m_Instance, "/user/hand/right", &m_Input.handSubactionPath[Side::RIGHT] );

        // Create actions.
        {
            // Create an input action for grabbing objects with the left and right hands.
            XrActionCreateInfo actionInfo{ XR_TYPE_ACTION_CREATE_INFO };
            actionInfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
            strcpy_s( actionInfo.actionName, "grab_object" );
            strcpy_s( actionInfo.localizedActionName, "Grab Object" );
            actionInfo.countSubactionPaths = uint32_t( m_Input.handSubactionPath.size() );
            actionInfo.subactionPaths = m_Input.handSubactionPath.data();
            xrCreateAction( m_Input.actionSet, &actionInfo, &m_Input.grabAction );

            // Create an input action getting the left and right hand poses.
            actionInfo.actionType = XR_ACTION_TYPE_POSE_INPUT;
            strcpy_s( actionInfo.actionName, "hand_pose" );
            strcpy_s( actionInfo.localizedActionName, "Hand Pose" );
            actionInfo.countSubactionPaths = uint32_t( m_Input.handSubactionPath.size() );
            actionInfo.subactionPaths = m_Input.handSubactionPath.data();
            xrCreateAction( m_Input.actionSet, &actionInfo, &m_Input.poseAction );

            // Create an input action getting the left joysticks.
            actionInfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
            strcpy_s( actionInfo.actionName, "left_thumbstick_x" );
            strcpy_s( actionInfo.localizedActionName, "Left Thumbstick X" );
            actionInfo.countSubactionPaths = uint32_t( m_Input.handSubactionPath.size() );
            actionInfo.subactionPaths = m_Input.handSubactionPath.data();
            xrCreateAction( m_Input.actionSet, &actionInfo, &m_Input.leftThumbstickXAction );

            actionInfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
            strcpy_s( actionInfo.actionName, "left_thumbstick_y" );
            strcpy_s( actionInfo.localizedActionName, "Left Thumbstick Y" );
            actionInfo.countSubactionPaths = uint32_t( m_Input.handSubactionPath.size() );
            actionInfo.subactionPaths = m_Input.handSubactionPath.data();
            xrCreateAction( m_Input.actionSet, &actionInfo, &m_Input.leftThumbstickYAction );

            // Create an input action getting the right joysticks.
            actionInfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
            strcpy_s( actionInfo.actionName, "right_thumbstick_x" );
            strcpy_s( actionInfo.localizedActionName, "Right Thumbstick X" );
            actionInfo.countSubactionPaths = uint32_t( m_Input.handSubactionPath.size() );
            actionInfo.subactionPaths = m_Input.handSubactionPath.data();
            xrCreateAction( m_Input.actionSet, &actionInfo, &m_Input.rightThumbstickXAction );

            // Create an input action getting the left and right joysticks.
            actionInfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
            strcpy_s( actionInfo.actionName, "right_thumbstick_y" );
            strcpy_s( actionInfo.localizedActionName, "Right Thumbstick Y" );
            actionInfo.countSubactionPaths = uint32_t( m_Input.handSubactionPath.size() );
            actionInfo.subactionPaths = m_Input.handSubactionPath.data();
            xrCreateAction( m_Input.actionSet, &actionInfo, &m_Input.rightThumbstickYAction );

            // Create output actions for vibrating the left and right controller.
            actionInfo.actionType = XR_ACTION_TYPE_VIBRATION_OUTPUT;
            strcpy_s( actionInfo.actionName, "vibrate_hand" );
            strcpy_s( actionInfo.localizedActionName, "Vibrate Hand" );
            actionInfo.countSubactionPaths = uint32_t( m_Input.handSubactionPath.size() );
            actionInfo.subactionPaths = m_Input.handSubactionPath.data();
            xrCreateAction( m_Input.actionSet, &actionInfo, &m_Input.vibrateAction );

            // Create input actions for quitting the session using the left and right controller.
            // Since it doesn't matter which hand did this, we do not specify subaction paths for it.
            // We will just suggest bindings for both hands, where possible.
            actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
            strcpy_s( actionInfo.actionName, "quit_session" );
            strcpy_s( actionInfo.localizedActionName, "Quit Session" );
            actionInfo.countSubactionPaths = 0;
            actionInfo.subactionPaths = nullptr;
            xrCreateAction( m_Input.actionSet, &actionInfo, &m_Input.quitAction ) ;
        }

        std::array<XrPath, Side::COUNT> selectPath;
        std::array<XrPath, Side::COUNT> squeezeValuePath;
        std::array<XrPath, Side::COUNT> squeezeForcePath;
        std::array<XrPath, Side::COUNT> squeezeClickPath;
        std::array<XrPath, Side::COUNT> posePath;
        std::array<XrPath, Side::COUNT> hapticPath;
        std::array<XrPath, Side::COUNT> menuClickPath;
        std::array<XrPath, Side::COUNT> bClickPath;
        std::array<XrPath, Side::COUNT> triggerValuePath;
        std::array<XrPath, Side::COUNT> leftThumbstickXPath;
        std::array<XrPath, Side::COUNT> leftThumbstickYPath;
        std::array<XrPath, Side::COUNT> rightThumbstickXPath;
        std::array<XrPath, Side::COUNT> rightThumbstickYPath;
        ( xrStringToPath( m_Instance, "/user/hand/left/input/select/click", &selectPath[Side::LEFT] ) );
        ( xrStringToPath( m_Instance, "/user/hand/right/input/select/click", &selectPath[Side::RIGHT] ) );
        ( xrStringToPath( m_Instance, "/user/hand/left/input/squeeze/value", &squeezeValuePath[Side::LEFT] ) );
        ( xrStringToPath( m_Instance, "/user/hand/right/input/squeeze/value", &squeezeValuePath[Side::RIGHT] ) );
        ( xrStringToPath( m_Instance, "/user/hand/left/input/squeeze/force", &squeezeForcePath[Side::LEFT] ) );
        ( xrStringToPath( m_Instance, "/user/hand/right/input/squeeze/force", &squeezeForcePath[Side::RIGHT] ) );
        ( xrStringToPath( m_Instance, "/user/hand/left/input/squeeze/click", &squeezeClickPath[Side::LEFT] ) );
        ( xrStringToPath( m_Instance, "/user/hand/right/input/squeeze/click", &squeezeClickPath[Side::RIGHT] ) );
        ( xrStringToPath( m_Instance, "/user/hand/left/input/grip/pose", &posePath[Side::LEFT] ) );
        ( xrStringToPath( m_Instance, "/user/hand/right/input/grip/pose", &posePath[Side::RIGHT] ) );
        ( xrStringToPath( m_Instance, "/user/hand/left/output/haptic", &hapticPath[Side::LEFT] ) );
        ( xrStringToPath( m_Instance, "/user/hand/right/output/haptic", &hapticPath[Side::RIGHT] ) );
        ( xrStringToPath( m_Instance, "/user/hand/left/input/menu/click", &menuClickPath[Side::LEFT] ) );
        ( xrStringToPath( m_Instance, "/user/hand/right/input/menu/click", &menuClickPath[Side::RIGHT] ) );
        ( xrStringToPath( m_Instance, "/user/hand/left/input/b/click", &bClickPath[Side::LEFT] ) );
        ( xrStringToPath( m_Instance, "/user/hand/right/input/b/click", &bClickPath[Side::RIGHT] ) );
        ( xrStringToPath( m_Instance, "/user/hand/left/input/trigger/value", &triggerValuePath[Side::LEFT] ) );
        ( xrStringToPath( m_Instance, "/user/hand/right/input/trigger/value", &triggerValuePath[Side::RIGHT] ) );
        ( xrStringToPath( m_Instance, "/user/hand/left/input/thumbstick/x", &leftThumbstickXPath[Side::LEFT] ) );
        ( xrStringToPath( m_Instance, "/user/hand/left/input/thumbstick/y", &leftThumbstickYPath[Side::LEFT] ));
        ( xrStringToPath( m_Instance, "/user/hand/right/input/thumbstick/x", &rightThumbstickXPath[Side::RIGHT] ) );
        ( xrStringToPath( m_Instance, "/user/hand/right/input/thumbstick/y", &rightThumbstickYPath[Side::RIGHT] ));
        // Suggest bindings for KHR Simple.
        {
            XrPath khrSimpleInteractionProfilePath;
            (
                xrStringToPath( m_Instance, "/interaction_profiles/khr/simple_controller", &khrSimpleInteractionProfilePath ) );
            std::vector<XrActionSuggestedBinding> bindings{ {// Fall back to a click input for the grab action.
                                                            {m_Input.grabAction, selectPath[Side::LEFT]},
                                                            {m_Input.grabAction, selectPath[Side::RIGHT]},
                                                            {m_Input.poseAction, posePath[Side::LEFT]},
                                                            {m_Input.poseAction, posePath[Side::RIGHT]},
                                                            {m_Input.quitAction, menuClickPath[Side::LEFT]},
                                                            {m_Input.quitAction, menuClickPath[Side::RIGHT]},
                                                            {m_Input.vibrateAction, hapticPath[Side::LEFT]},
                                                            {m_Input.vibrateAction, hapticPath[Side::RIGHT]}, 
                                                            {m_Input.leftThumbstickXAction, leftThumbstickXPath[Side::LEFT]},
                                                            {m_Input.leftThumbstickYAction, leftThumbstickYPath[Side::LEFT]},
                                                            {m_Input.rightThumbstickXAction, rightThumbstickXPath[Side::RIGHT]},
                                                            {m_Input.rightThumbstickYAction, rightThumbstickYPath[Side::RIGHT]}}};
            XrInteractionProfileSuggestedBinding suggestedBindings{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
            suggestedBindings.interactionProfile = khrSimpleInteractionProfilePath;
            suggestedBindings.suggestedBindings = bindings.data();
            suggestedBindings.countSuggestedBindings = (uint32_t)bindings.size();
            ( xrSuggestInteractionProfileBindings( m_Instance, &suggestedBindings ) );
        }
        // Suggest bindings for the Oculus Touch.
        {
            XrPath oculusTouchInteractionProfilePath;
            (
                xrStringToPath( m_Instance, "/interaction_profiles/oculus/touch_controller", &oculusTouchInteractionProfilePath ));
            std::vector<XrActionSuggestedBinding> bindings{ {
                                                            {m_Input.grabAction, squeezeValuePath[Side::LEFT]},
                                                            {m_Input.grabAction, squeezeValuePath[Side::RIGHT]},
                                                            {m_Input.poseAction, posePath[Side::LEFT]},
                                                            {m_Input.poseAction, posePath[Side::RIGHT]},
                                                            {m_Input.quitAction, menuClickPath[Side::LEFT]},
                                                            {m_Input.vibrateAction, hapticPath[Side::LEFT]},
                                                            {m_Input.vibrateAction, hapticPath[Side::RIGHT]},
                                                            {m_Input.leftThumbstickXAction, leftThumbstickXPath[Side::LEFT]},
                                                            {m_Input.leftThumbstickYAction, leftThumbstickYPath[Side::LEFT]},
                                                            {m_Input.rightThumbstickXAction, rightThumbstickXPath[Side::RIGHT]},
                                                            {m_Input.rightThumbstickYAction, rightThumbstickYPath[Side::RIGHT]}} };
            XrInteractionProfileSuggestedBinding suggestedBindings{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
            suggestedBindings.interactionProfile = oculusTouchInteractionProfilePath;
            suggestedBindings.suggestedBindings = bindings.data();
            suggestedBindings.countSuggestedBindings = (uint32_t)bindings.size();
            ( xrSuggestInteractionProfileBindings( m_Instance, &suggestedBindings ) );
        }
        // Suggest bindings for the Vive Controller.
        {
            XrPath viveControllerInteractionProfilePath;
            (
                xrStringToPath( m_Instance, "/interaction_profiles/htc/vive_controller", &viveControllerInteractionProfilePath ) );
            std::vector<XrActionSuggestedBinding> bindings{ {{m_Input.grabAction, triggerValuePath[Side::LEFT]},
                                                            {m_Input.grabAction, triggerValuePath[Side::RIGHT]},
                                                            {m_Input.poseAction, posePath[Side::LEFT]},
                                                            {m_Input.poseAction, posePath[Side::RIGHT]},
                                                            {m_Input.quitAction, menuClickPath[Side::LEFT]},
                                                            {m_Input.quitAction, menuClickPath[Side::RIGHT]},
                                                            {m_Input.vibrateAction, hapticPath[Side::LEFT]},
                                                            {m_Input.vibrateAction, hapticPath[Side::RIGHT]},
                                                            {m_Input.leftThumbstickXAction, leftThumbstickXPath[Side::LEFT]},
                                                            {m_Input.leftThumbstickYAction, leftThumbstickYPath[Side::LEFT]},
                                                            {m_Input.rightThumbstickXAction, rightThumbstickXPath[Side::RIGHT]},
                                                            {m_Input.rightThumbstickYAction, rightThumbstickYPath[Side::RIGHT]}} };
            XrInteractionProfileSuggestedBinding suggestedBindings{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
            suggestedBindings.interactionProfile = viveControllerInteractionProfilePath;
            suggestedBindings.suggestedBindings = bindings.data();
            suggestedBindings.countSuggestedBindings = (uint32_t)bindings.size();
            ( xrSuggestInteractionProfileBindings( m_Instance, &suggestedBindings ) );
        }

        // Suggest bindings for the Valve Index Controller.
        {
            XrPath indexControllerInteractionProfilePath;
            (
                xrStringToPath( m_Instance, "/interaction_profiles/valve/index_controller", &indexControllerInteractionProfilePath ));
            std::vector<XrActionSuggestedBinding> bindings{ {{m_Input.grabAction, squeezeForcePath[Side::LEFT]},
                                                            {m_Input.grabAction, squeezeForcePath[Side::RIGHT]},
                                                            {m_Input.poseAction, posePath[Side::LEFT]},
                                                            {m_Input.poseAction, posePath[Side::RIGHT]},
                                                            {m_Input.quitAction, bClickPath[Side::LEFT]},
                                                            {m_Input.quitAction, bClickPath[Side::RIGHT]},
                                                            {m_Input.vibrateAction, hapticPath[Side::LEFT]},
                                                            {m_Input.vibrateAction, hapticPath[Side::RIGHT]},
                                                            {m_Input.leftThumbstickXAction, leftThumbstickXPath[Side::LEFT]},
                                                            {m_Input.leftThumbstickYAction, leftThumbstickYPath[Side::LEFT]},
                                                            {m_Input.rightThumbstickXAction, rightThumbstickXPath[Side::RIGHT]},
                                                            {m_Input.rightThumbstickYAction, rightThumbstickYPath[Side::RIGHT]}} };
            XrInteractionProfileSuggestedBinding suggestedBindings{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
            suggestedBindings.interactionProfile = indexControllerInteractionProfilePath;
            suggestedBindings.suggestedBindings = bindings.data();
            suggestedBindings.countSuggestedBindings = (uint32_t)bindings.size();
            ( xrSuggestInteractionProfileBindings( m_Instance, &suggestedBindings ) );
        }

        // Suggest bindings for the Microsoft Mixed Reality Motion Controller.
        {
            XrPath microsoftMixedRealityInteractionProfilePath;
            ( xrStringToPath( m_Instance, "/interaction_profiles/microsoft/motion_controller",
                &microsoftMixedRealityInteractionProfilePath ) );
            std::vector<XrActionSuggestedBinding> bindings{ {{m_Input.grabAction, squeezeClickPath[Side::LEFT]},
                                                            {m_Input.grabAction, squeezeClickPath[Side::RIGHT]},
                                                            {m_Input.poseAction, posePath[Side::LEFT]},
                                                            {m_Input.poseAction, posePath[Side::RIGHT]},
                                                            {m_Input.quitAction, menuClickPath[Side::LEFT]},
                                                            {m_Input.quitAction, menuClickPath[Side::RIGHT]},
                                                            {m_Input.vibrateAction, hapticPath[Side::LEFT]},
                                                            {m_Input.vibrateAction, hapticPath[Side::RIGHT]},
                                                            {m_Input.leftThumbstickXAction, leftThumbstickXPath[Side::LEFT]},
                                                            {m_Input.leftThumbstickYAction, leftThumbstickYPath[Side::LEFT]},
                                                            {m_Input.rightThumbstickXAction, rightThumbstickXPath[Side::RIGHT]},
                                                            {m_Input.rightThumbstickYAction, rightThumbstickYPath[Side::RIGHT]}} };
            XrInteractionProfileSuggestedBinding suggestedBindings{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
            suggestedBindings.interactionProfile = microsoftMixedRealityInteractionProfilePath;
            suggestedBindings.suggestedBindings = bindings.data();
            suggestedBindings.countSuggestedBindings = (uint32_t)bindings.size();
            ( xrSuggestInteractionProfileBindings( m_Instance, &suggestedBindings ) );
        }
        XrActionSpaceCreateInfo actionSpaceInfo{ XR_TYPE_ACTION_SPACE_CREATE_INFO };
        actionSpaceInfo.action = m_Input.poseAction;
        actionSpaceInfo.poseInActionSpace.orientation.w = 1.f;
        actionSpaceInfo.subactionPath = m_Input.handSubactionPath[Side::LEFT];
        ( xrCreateActionSpace( m_Session, &actionSpaceInfo, &m_Input.handSpace[Side::LEFT] ) );
        actionSpaceInfo.subactionPath = m_Input.handSubactionPath[Side::RIGHT];
        ( xrCreateActionSpace( m_Session, &actionSpaceInfo, &m_Input.handSpace[Side::RIGHT] ) );

        XrSessionActionSetsAttachInfo attachInfo{ XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
        attachInfo.countActionSets = 1;
        attachInfo.actionSets = &m_Input.actionSet;
        ( xrAttachSessionActionSets( m_Session, &attachInfo ) );
    }
    void XRCore::CreateVisualizedSpaces()
    {
        std::string visualizedSpaces[] = { "ViewFront",        "Local", "Stage", "StageLeft", "StageRight", "StageLeftRotated",
                                         "StageRightRotated" };

        for (const auto& visualizedSpace : visualizedSpaces) 
        {
            XrReferenceSpaceCreateInfo referenceSpaceCreateInfo = GetXrReferenceSpaceCreateInfo( visualizedSpace );
            XrSpace space;
            XrResult res = xrCreateReferenceSpace( m_Session, &referenceSpaceCreateInfo, &space );
            if (XR_SUCCEEDED( res )) 
            {
                m_visualizedSpaces.push_back( space );
            }
            else 
            {
                KM_CORE_WARN( "Failed to create reference space {} with error {}", visualizedSpace.c_str(), res );
            }
        }
    }
    const XrEventDataBaseHeader* XRCore::TryReadNextEvent()
    {
        // It is sufficient to clear the just the XrEventDataBuffer header to
                // XR_TYPE_EVENT_DATA_BUFFER
        XrEventDataBaseHeader* baseHeader = reinterpret_cast<XrEventDataBaseHeader*>(&m_eventDataBuffer);
        *baseHeader = { XR_TYPE_EVENT_DATA_BUFFER };
        const XrResult xr = xrPollEvent( m_Instance, &m_eventDataBuffer );
        if (xr == XR_SUCCESS) 
        {
            if (baseHeader->type == XR_TYPE_EVENT_DATA_EVENTS_LOST) 
            {
                const XrEventDataEventsLost* const eventsLost = reinterpret_cast<const XrEventDataEventsLost*>(baseHeader);
                KM_CORE_TRACE( "{} events lost", sizeof(eventsLost) );
            }

            return baseHeader;
        }
        if (xr == XR_EVENT_UNAVAILABLE) 
        {
            return nullptr;
        }
        KM_CORE_ERROR( "Shouldn't be here." );
        return {};
    }
    void XRCore::HandleSessionStateChangedEvent( const XrEventDataSessionStateChanged& stateChangedEvent, bool* exitRenderLoop, bool* requestRestart )
    {
        const XrSessionState oldState = m_sessionState;
        m_sessionState = stateChangedEvent.state;

        KM_CORE_TRACE( "XrEventDataSessionStateChanged: state {}->{} session={} time={}", to_string( oldState ),
           to_string( m_sessionState ), "m_Session", stateChangedEvent.time);

        if ((stateChangedEvent.session != XR_NULL_HANDLE) && (stateChangedEvent.session != m_Session)) {
            KM_CORE_ERROR( "XrEventDataSessionStateChanged for unknown session" );
            return;
        }

        switch (m_sessionState) 
        {
        case XR_SESSION_STATE_READY: 
        {
            XrSessionBeginInfo sessionBeginInfo{ XR_TYPE_SESSION_BEGIN_INFO };
            sessionBeginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
            xrBeginSession( m_Session, &sessionBeginInfo );
            m_sessionRunning = true;
            break;
        }
        case XR_SESSION_STATE_STOPPING: 
        {
            m_sessionRunning = false;
            xrEndSession( m_Session );
            break;
        }
        case XR_SESSION_STATE_EXITING: 
        {
            *exitRenderLoop = true;
            // Do not attempt to restart because user closed this session.
            *requestRestart = false;
            break;
        }
        case XR_SESSION_STATE_LOSS_PENDING:
        {
            *exitRenderLoop = true;
            // Poll for a new instance.
            *requestRestart = true;
            break;
        }
        default:
            break;
        }
    }
    void XRCore::LogActionSourceName( XrAction action, const std::string& actionName ) const
    {
        XrBoundSourcesForActionEnumerateInfo getInfo = { XR_TYPE_BOUND_SOURCES_FOR_ACTION_ENUMERATE_INFO };
        getInfo.action = action;
        uint32_t pathCount = 0;
        xrEnumerateBoundSourcesForAction( m_Session, &getInfo, 0, &pathCount, nullptr );
        std::vector<XrPath> paths( pathCount );
        xrEnumerateBoundSourcesForAction( m_Session, &getInfo, uint32_t( paths.size() ), &pathCount, paths.data() );

        std::string sourceName;
        for (uint32_t i = 0; i < pathCount; ++i)
        {
            constexpr XrInputSourceLocalizedNameFlags all = XR_INPUT_SOURCE_LOCALIZED_NAME_USER_PATH_BIT |
                XR_INPUT_SOURCE_LOCALIZED_NAME_INTERACTION_PROFILE_BIT |
                XR_INPUT_SOURCE_LOCALIZED_NAME_COMPONENT_BIT;

            XrInputSourceLocalizedNameGetInfo nameInfo = { XR_TYPE_INPUT_SOURCE_LOCALIZED_NAME_GET_INFO };
            nameInfo.sourcePath = paths[i];
            nameInfo.whichComponents = all;

            uint32_t size = 0;
            xrGetInputSourceLocalizedName( m_Session, &nameInfo, 0, &size, nullptr );
            if (size < 1) 
            {
                continue;
            }
            std::vector<char> grabSource( size );
            xrGetInputSourceLocalizedName( m_Session, &nameInfo, uint32_t( grabSource.size() ), &size, grabSource.data() );
            if (!sourceName.empty()) {
                sourceName += " and ";
            }
            sourceName += "'";
            sourceName += std::string( grabSource.data(), size - 1 );
            sourceName += "'";
        }

        KM_CORE_INFO( "{} action is bound to {}", actionName.c_str(), ((!sourceName.empty()) ? sourceName.c_str() : "nothing") );
    }

    bool XRCore::RenderLayer( XrTime predictedDisplayTime, std::vector<XrCompositionLayerProjectionView>& projectionLayerViews, XrCompositionLayerProjection& layer )
    {
        XrViewState viewState{ XR_TYPE_VIEW_STATE };
        uint32_t viewCapacityInput = (uint32_t)m_views.size();
        uint32_t viewCountOutput;

        XrViewLocateInfo viewLocateInfo{ XR_TYPE_VIEW_LOCATE_INFO };
        viewLocateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
        viewLocateInfo.displayTime = predictedDisplayTime;
        viewLocateInfo.space = m_Space;

        m_PredictedDisplayTime = predictedDisplayTime;

        m_LastCallResult = xrLocateViews( m_Session, &viewLocateInfo, &viewState, viewCapacityInput, &viewCountOutput, m_views.data() );
        if ((viewState.viewStateFlags & XR_VIEW_STATE_POSITION_VALID_BIT) == 0 ||
            (viewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT) == 0) 
        {
            return false;  // There is no valid tracking poses for the views.
        }

        projectionLayerViews.resize( viewCountOutput );

        // Render view to the appropriate part of the swapchain image.
        for (uint32_t i = 0; i < viewCountOutput; i++) 
        {
            // Each view has a separate swapchain which is acquired, rendered to, and released.
            const Swapchain viewSwapchain = m_swapchains[i];

            XrSwapchainImageAcquireInfo acquireInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };

            uint32_t swapchainImageIndex;
            xrAcquireSwapchainImage( viewSwapchain.handle, &acquireInfo, &swapchainImageIndex );

            XrSwapchainImageWaitInfo waitInfo{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
            waitInfo.timeout = XR_INFINITE_DURATION;
            xrWaitSwapchainImage( viewSwapchain.handle, &waitInfo );

            XrPosef translatedPos = m_views[i].pose;
            XrVector3f_Add( &translatedPos.position, &m_views[i].pose.position, &m_Pose.position );

            projectionLayerViews[i] = { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW };
            projectionLayerViews[i].pose = translatedPos;
            projectionLayerViews[i].fov = m_views[i].fov;
            projectionLayerViews[i].subImage.swapchain = viewSwapchain.handle;
            projectionLayerViews[i].subImage.imageRect.offset = { 0, 0 };
            projectionLayerViews[i].subImage.imageRect.extent = { viewSwapchain.width, viewSwapchain.height };

            const XrSwapchainImageBaseHeader* const swapchainImage = m_swapchainImages[viewSwapchain.handle][swapchainImageIndex];
            gfx->RenderView( projectionLayerViews[i], swapchainImage, m_colorSwapchainFormat );

            XrSwapchainImageReleaseInfo releaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
            xrReleaseSwapchainImage( viewSwapchain.handle, &releaseInfo );
        }

        layer.space = m_Space;
        layer.layerFlags =
            XR_ENVIRONMENT_BLEND_MODE_OPAQUE == XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND
            ? XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT | XR_COMPOSITION_LAYER_UNPREMULTIPLIED_ALPHA_BIT
            : 0;
        layer.viewCount = (uint32_t)projectionLayerViews.size();
        layer.views = projectionLayerViews.data();
        return true;
    }
}