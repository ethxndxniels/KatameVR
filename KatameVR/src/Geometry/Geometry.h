#pragma once

#include <openxr/openxr.h>

namespace Geometry {

    struct CubeVertex 
	{
        XrVector3f Position;
        XrVector3f Color;
    };

    constexpr XrVector3f Red{ 1, 0, 0 };
    constexpr XrVector3f DarkRed{ 0.25f, 0, 0 };
    constexpr XrVector3f Green{ 0, 1, 0 };
    constexpr XrVector3f DarkGreen{ 0, 0.25f, 0 };
    constexpr XrVector3f Blue{ 0, 0, 1 };
    constexpr XrVector3f DarkBlue{ 0, 0, 0.25f };

	static std::vector<CubeVertex> CreateCubeVertices()
	{
		constexpr float side = 1.0f / 2.0f;

		return std::vector<CubeVertex>
		{
			{ { -side, -side, -side }, { 0, 0, 255 } },
			{ { side, -side, -side }, { 0, 255, 0 } },
			{ { side, side, -side }, { 255, 0, 0 } },
			{ { -side, side, -side }, { 0, 255, 255 } },
			{ { -side, -side, side }, { 0, 0, 255 } },
			{ { side, -side, side }, { 255, 0, 0 } },
			{ { side, side, side }, { 0, 255, 0 } },
			{ { -side, side, side }, { 0, 255, 255 } },
		};
	}

	static std::vector<unsigned int> CreateCubeIndices()
	{
		return std::vector<unsigned int>
		{
			1, 2, 0, 2, 3, 0, 4, 6, 5, 7, 6, 4,
			6, 2, 1, 5, 6, 1, 3, 7, 4, 0, 3, 4,
			4, 5, 1, 0, 4, 1, 2, 7, 3, 2, 6, 7,
		};
	}

	static std::vector<D3D11_INPUT_ELEMENT_DESC> CreateCubeInputLayout()
	{
		return
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}

}