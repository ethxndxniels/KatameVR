#pragma once

#include "XRCore.h"
#include "XRCommon.h"

#include <DirectXMath.h>

namespace Katame
{
	// TODO: Clean Up XR Code. There is way too much that is not being used. It looks like rendering needs to be done in RenderLayer
	// so I will need to add in Submissions to a render queue from Application. i.e. ( XRRender::Submit( m_Model ) ); or
	// XRRender::Submit( m_Model->GetBindables() );

	class XRRender
	{
	public:
		enum EMaskType
		{
			MASK_VISIBLE = 0,
			MASK_HIDDEN = 1,
			MASK_LINE_LOOP = 2
		};

		static const uint32_t k_nVRViewCount = 2;
		static const float k_fMedianIPD;
		static const float k_fMinIPD;
		static const float k_fTau;
	public:
		static void Init();
		static void Destroy();
	public:
		static void BeginFrame( unsigned int index );
		static void RenderFrame();
		static bool RenderLayer( XrTime predictedDisplayTime, std::vector<XrCompositionLayerProjectionView>& projectionLayerViews, XrCompositionLayerProjection& layer );
	public:
		static DirectX::XMMATRIX GetView( unsigned int index );
		static DirectX::XMMATRIX GetProjection( unsigned int index );
	public:
		static std::vector<XrView> GetViews();
		static std::vector<XrViewConfigurationView> GetConfigViews();
		static bool GetDepthHandling();
		static float GetCurrentIPD();
		static XrPosef GetHMDPose();
		static XRHMDState* GetHMDState();
		static std::vector<XrSwapchain> GetSwapchainColor();
		static std::vector<XrSwapchain> GetSwapchainDepth();
		static XrTime GetPredictedDisplayTime();
		static XrDuration GetPredictedDisplayPeriod();
		static uint32_t GetTextureArraySize();
		static int64_t GetTextureFormat();
		static uint32_t GetTextureHeight();
		static uint32_t GetTextureMipCount();
		static uint32_t GetTextureWidth();
		static bool GetVisibilityMask( EXREye eEye, EMaskType eMaskType, std::vector< float >& vMaskVertices, std::vector< uint32_t >& vMaskIndices );
	public:
		static void GenerateSwapchains( bool bIsDepth = false );
		static void DestroySwapchains( std::vector< XrSwapchain >& vXRSwapchains );
		static void ResetHMDState();
		static void SetHMDState( EXREye eEye, XREyeState* pEyeState );
		static void SetSwapchainFormat( std::vector< int64_t > vAppTextureFormats, std::vector< int64_t > vAppDepthFormats );
	private:
		static ID3D11DepthStencilView* m_DSV;
		static ID3D11RenderTargetView* m_RTV;
	private:
		static XRHMDState* m_HMDState;
		static XrResult m_LastCallResult;
		static XrTime m_PredictedDisplayTime;
		static XrDuration m_PredictedDisplayPeriod;
	private:
		static bool m_bDepthHandling;
		static uint32_t u_TextureArraySize;
		static uint32_t u_TextureMipCount;
		static uint32_t u_TextureWidth;
		static uint32_t u_TextureHeight;
		static int64_t i_TextureFormat;
		static int64_t i_DepthFormat;
		static std::vector<XrSwapchain> m_SwapChainsColor;
		static std::vector<XrSwapchain> m_SwapChainsDepth;
		static std::vector<XrView> m_Views;
		static std::vector<XrViewConfigurationView> m_ViewConfigs;
		static std::vector<XrSpace> m_VisualizedSpaces;
	};
}