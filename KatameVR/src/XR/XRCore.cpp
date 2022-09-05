#include "XRCore.h"
#include "../Core/Log.h"

#include "../Core/Application.h"

namespace Katame
{
	void XRCore::CreateInstance()
	{
		LogLayersAndExtensions();

		CreateInstanceInternal();

		LogInstanceInfo();
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
                KM_CORE_INFO( "Name={} SpecVersion={} LayerVersion={} Description={}", layer.layerName,  layer.specVersion, 
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
        const std::vector<std::string> platformExtensions = m_platformPlugin->GetInstanceExtensions();
        std::transform( platformExtensions.begin(), platformExtensions.end(), std::back_inserter( extensions ),
            []( const std::string& ext ) { return ext.c_str(); } );
        const std::vector<std::string> graphicsExtensions = m_graphicsPlugin->GetInstanceExtensions();
        std::transform( graphicsExtensions.begin(), graphicsExtensions.end(), std::back_inserter( extensions ),
            []( const std::string& ext ) { return ext.c_str(); } );

        XrInstanceCreateInfo createInfo{ XR_TYPE_INSTANCE_CREATE_INFO };
        createInfo.next = m_platformPlugin->GetInstanceCreateExtension();
        createInfo.enabledExtensionCount = (uint32_t)extensions.size();
        createInfo.enabledExtensionNames = extensions.data();

        strcpy( createInfo.applicationInfo.applicationName, "HelloXR" );
        createInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;

        xrCreateInstance( &createInfo, &m_Instance );
    }
}