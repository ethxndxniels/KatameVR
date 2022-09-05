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

        namespace Side
        {
            const int LEFT = 0;
            const int RIGHT = 1;
            const int COUNT = 2;
        }

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
            KM_CORE_ERROR( "Unknown reference space type {}", referenceSpaceTypeStr.c_str() );
            throw std::exception{};
        }
        return referenceSpaceCreateInfo;
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
        //m_graphicsPlugin->InitializeDevice( m_Instance, m_SystemId );
    }

    void XRCore::InitializeSession() 
    {
        {
            KM_CORE_INFO( "Creating session..." );

            XrSessionCreateInfo createInfo{ XR_TYPE_SESSION_CREATE_INFO };
            createInfo.next = reinterpret_cast<const XrBaseInStructure*>(XR_TYPE_GRAPHICS_BINDING_D3D11_KHR);
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

    void LogLayersAndExtensions()
    {
        // Write out extension properties for a given layer.
        const auto logExtensions = []( const char* layerName, int indent = 0 ) {
            uint32_t instanceExtensionCount;
            xrEnumerateInstanceExtensionProperties( layerName, 0, &instanceExtensionCount, nullptr );

            std::vector<XrExtensionProperties> extensions( instanceExtensionCount );
            for (XrExtensionProperties& extension : extensions) {
                extension.type = XR_TYPE_EXTENSION_PROPERTIES;
            }

            xrEnumerateInstanceExtensionProperties( layerName, (uint32_t)extensions.size(), &instanceExtensionCount, extensions.data() );

            KM_CORE_INFO( "Available Extensions: {}", instanceExtensionCount );
            for (const XrExtensionProperties& extension : extensions)
            {
                KM_CORE_INFO( "Name={} SpecVersion={}", extension.extensionName, extension.extensionVersion );
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
                KM_CORE_INFO( "Name={} SpecVersion={} LayerVersion={} Description={}", layer.layerName, layer.specVersion,
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

        strcpy( createInfo.applicationInfo.applicationName, "HelloXR" );
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
            KM_CORE_INFO( "  View Configuration Type: {} {}", std::to_string( viewConfigType ), viewConfigType == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO ? "(Selected)" : "" );

            XrViewConfigurationProperties viewConfigProperties{ XR_TYPE_VIEW_CONFIGURATION_PROPERTIES };
            xrGetViewConfigurationProperties( m_Instance, m_SystemId, viewConfigType, &viewConfigProperties );

            KM_CORE_INFO( "  View configuration FovMutable={}", viewConfigProperties.fovMutable == XR_TRUE ? "True" : "False" );

            uint32_t viewCount;
            xrEnumerateViewConfigurationViews( m_Instance, m_SystemId, viewConfigType, 0, &viewCount, nullptr );
            if (viewCount > 0)
            {
                std::vector<XrViewConfigurationView> views( viewCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW } );
                xrEnumerateViewConfigurationViews( m_Instance, m_SystemId, viewConfigType, viewCount, &viewCount, views.data() );

                for (uint32_t i = 0; i < views.size(); i++) {
                    const XrViewConfigurationView& view = views[i];

                    KM_CORE_INFO( "    View [%d]: Recommended Width=%d Height=%d SampleCount=%d", i,
                        view.recommendedImageRectWidth, view.recommendedImageRectHeight,
                        view.recommendedSwapchainSampleCount );
                    KM_CORE_INFO( "    View [%d]:     Maximum Width=%d Height=%d SampleCount=%d", i, view.maxImageRectWidth,
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
            KM_CORE_INFO( "Environment Blend Mode {} : {}", std::to_string( mode ), blendModeMatch ? "(Selected)" : "" );
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
            KM_CORE_INFO( "  Name: {}", std::to_string(space) );
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
                                                            {m_Input.vibrateAction, hapticPath[Side::RIGHT]}} };
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
                xrStringToPath( m_Instance, "/interaction_profiles/oculus/touch_controller", &oculusTouchInteractionProfilePath ) );
            std::vector<XrActionSuggestedBinding> bindings{ {{m_Input.grabAction, squeezeValuePath[Side::LEFT]},
                                                            {m_Input.grabAction, squeezeValuePath[Side::RIGHT]},
                                                            {m_Input.poseAction, posePath[Side::LEFT]},
                                                            {m_Input.poseAction, posePath[Side::RIGHT]},
                                                            {m_Input.quitAction, menuClickPath[Side::LEFT]},
                                                            {m_Input.vibrateAction, hapticPath[Side::LEFT]},
                                                            {m_Input.vibrateAction, hapticPath[Side::RIGHT]}} };
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
                                                            {m_Input.vibrateAction, hapticPath[Side::RIGHT]}} };
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
                xrStringToPath( m_Instance, "/interaction_profiles/valve/index_controller", &indexControllerInteractionProfilePath ) );
            std::vector<XrActionSuggestedBinding> bindings{ {{m_Input.grabAction, squeezeForcePath[Side::LEFT]},
                                                            {m_Input.grabAction, squeezeForcePath[Side::RIGHT]},
                                                            {m_Input.poseAction, posePath[Side::LEFT]},
                                                            {m_Input.poseAction, posePath[Side::RIGHT]},
                                                            {m_Input.quitAction, bClickPath[Side::LEFT]},
                                                            {m_Input.quitAction, bClickPath[Side::RIGHT]},
                                                            {m_Input.vibrateAction, hapticPath[Side::LEFT]},
                                                            {m_Input.vibrateAction, hapticPath[Side::RIGHT]}} };
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
                                                            {m_Input.vibrateAction, hapticPath[Side::RIGHT]}} };
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
}