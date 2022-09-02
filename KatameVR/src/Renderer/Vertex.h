#pragma once

#include <DirectXMath.h>

struct VertexPosNormTanBiTex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT3 Tangent;
	DirectX::XMFLOAT3 Binormal;
	DirectX::XMFLOAT2 Texcoord;
};

struct VertexPosColor
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Color;
};