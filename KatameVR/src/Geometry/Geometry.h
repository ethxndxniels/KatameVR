#pragma once

#include <openxr/openxr.h>

namespace Geometry {

    struct CubeVertex {
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
		return std::vector<CubeVertex>
		{
			{ { -3.0f, 3.0f, -3.0f }, { 0, 0, 255 } },
			{ { 3.0f, 3.0f, -3.0f}, { 0, 255, 0 } },
			{ { -3.0f, -3.0f, -3.0f }, { 255, 0, 0 } },
			{ { 3.0f, -3.0f, -3.0f }, { 0, 255, 255 } },
			{ { -3.0f, 3.0f, 3.0f }, { 0, 0, 255 } },
			{ { 3.0f, 3.0f, 3.0f }, { 255, 0, 0 } },
			{ { -3.0f, -3.0f, 3.0f }, { 0, 255, 0 } },
			{ { 3.0f, -3.0f, 3.0f }, { 0, 255, 255 } },
		};
	}

	static std::vector<unsigned int> CreateCubeIndices()
	{
		return std::vector<unsigned int>
		{
			0, 1, 2,
				2, 1, 3,
				4, 0, 6,
				6, 0, 2,
				7, 5, 6,
				6, 5, 4,
				3, 1, 7,
				7, 1, 5,
				4, 5, 0,
				0, 5, 1,
				3, 7, 2,
				2, 7, 6,
		};;
	}

	static std::vector<D3D11_INPUT_ELEMENT_DESC> CreateCubeInputLayout()
	{
		return
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}

}