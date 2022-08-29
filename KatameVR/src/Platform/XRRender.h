#pragma once

#include "XRCore.h"

namespace Katame
{
	class XRRender
	{
		static const uint32_t k_nVRViewCount = 2;
		static const float k_fMedianIPD;
		static const float k_fMinIPD;
		static const float k_fTau;
	public:
		static void Init();
		static std::vector< XrViewConfigurationView > GetConfigViews() { return m_vXRViewConfigs; }
		static bool GetDepthHandling() { return m_bDepthHandling; }
		static XRGraphics* GetGraphics();
		static float GetCurrentIPD();
		static void GetEyeProjection( EXREye eEye, std::vector<float>* mProjectionMatrix, bool bInvert = false );
		static XrPosef GetHMDPose();
		static XRHMDState* GetHMDState() { return m_pXRHMDState; }
		static std::vector<XrSwapchain> GetSwapchainColor() { return m_vXRSwapChainsColor; }
		static std::vector<XrSwapchain> GetSwapchainDepth() { return m_vXRSwapChainsDepth; }
		static XrTime GetPredictedDisplayTime() const { return m_xrPredictedDisplayTime; }
		static XrDuration GetPredictedDisplayPeriod() const { return m_xrPredictedDisplayPeriod; }
		static uint32_t GetTextureArraySize() { return m_nTextureArraySize; }
		static int64_t GetTextureFormat() { return m_nTextureFormat; }
		static uint32_t GetTextureHeight() { return m_nTextureHeight; }
		static uint32_t GetTextureMipCount() { return m_nTextureMipCount; }
		static  GetTextureWidth() { return m_nTextureWidth; }
		static XRExtVisibilityMask* GetXRVisibilityMask() { return m_pXRVisibilityMask; }
		static bool ProcessXRFrame();
	private:
		static void GenerateSwapchains( bool bIsDepth = false );
		static void DestroySwapchains( std::vector< XrSwapchain >& vXRSwapchains );
		static void ResetHMDState();
		static void SetHMDState( EXREye eEye, XREyeState* pEyeState );
		static void SetSwapchainFormat( std::vector< int64_t > vAppTextureFormats, std::vector< int64_t > vAppDepthFormats );
	private:
		static bool m_bDepthHandling = false;
		static uint32_t m_nTextureArraySize = 1;
		static uint32_t m_nTextureMipCount = 1;
		static uint32_t m_nTextureWidth = 1920;
		static uint32_t m_nTextureHeight = 1080;
		static int64_t m_nTextureFormat = 0;
		static int64_t m_nDepthFormat = 0;
		static std::vector<XrSwapchain> m_vXRSwapChainsColor;
		static std::vector<XrSwapchain> m_vXRSwapChainsDepth;
		static std::vector<XrView> m_vXRViews;
		static std::vector<XrViewConfigurationView> m_vXRViewConfigs;
		static XRExtVisibilityMask* m_pXRVisibilityMask = nullptr;
		static XRHMDState* m_pXRHMDState = nullptr;
		static XrResult m_xrLastCallResult = XR_SUCCESS;
		static XrTime m_xrPredictedDisplayTime;
		static XrDuration m_xrPredictedDisplayPeriod;
	};
}
