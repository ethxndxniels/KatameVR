// Copyright (c) 2017-2022, The Khronos Group Inc.
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <DirectXMath.h>
#include "../Utilities/xr_linear.h"
#include <directxmath.h> // Matrix math functions and objects
#include <d3d11.h>

struct ModelConstantBuffer {
    DirectX::XMFLOAT4X4 Model;
};
struct ViewProjectionConstantBuffer {
    DirectX::XMFLOAT4X4 ViewProjection;
};

DirectX::XMMATRIX XM_CALLCONV LoadXrPose( const XrPosef& pose );
DirectX::XMMATRIX XM_CALLCONV LoadXrMatrix( const XrMatrix4x4f& matrix );

ID3DBlob* CompileShader( const char* hlsl, const char* entrypoint, const char* shaderTarget );
IDXGIAdapter1* GetAdapter( LUID adapterId );
