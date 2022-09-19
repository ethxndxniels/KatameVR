#pragma once

#include <openxr/openxr.h>

namespace Geometry 
{
	// Vertices for a 1x1x1 meter cube. (Left/Right, Top/Bottom, Front/Back)
	constexpr XrVector3f LBB{ -0.5f, -0.5f, -0.5f };
	constexpr XrVector3f LBF{ -0.5f, -0.5f, 0.5f };
	constexpr XrVector3f LTB{ -0.5f, 0.5f, -0.5f };
	constexpr XrVector3f LTF{ -0.5f, 0.5f, 0.5f };
	constexpr XrVector3f RBB{ 0.5f, -0.5f, -0.5f };
	constexpr XrVector3f RBF{ 0.5f, -0.5f, 0.5f };
	constexpr XrVector3f RTB{ 0.5f, 0.5f, -0.5f };
	constexpr XrVector3f RTF{ 0.5f, 0.5f, 0.5f };

#define CUBE_SIDE(V1, V2, V3, V4, V5, V6, COLOR) {V1, COLOR}, {V2, COLOR}, {V3, COLOR}, {V4, COLOR}, {V5, COLOR}, {V6, COLOR},


	/////////////////////////////////////////
	// ColorCube
	/////////////////////////////////////////

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

	static std::vector<CubeVertex> CreateColorCubeVertices()
	{

		return std::vector<CubeVertex>
		{
			CUBE_SIDE( LTB, LBF, LBB, LTB, LTF, LBF, DarkRed )    // -X
			CUBE_SIDE( RTB, RBB, RBF, RTB, RBF, RTF, Red )        // +X
			CUBE_SIDE( LBB, LBF, RBF, LBB, RBF, RBB, DarkGreen )  // -Y
			CUBE_SIDE( LTB, RTB, RTF, LTB, RTF, LTF, Green )      // +Y
			CUBE_SIDE( LBB, RBB, RTB, LBB, RTB, LTB, DarkBlue )   // -Z
			CUBE_SIDE( LBF, LTF, RTF, LBF, RTF, RBF, Blue )       // +Z
		};
	}

	static std::vector<unsigned short> CreateColorCubeIndices()
	{
		return std::vector<unsigned short>
		{
			0, 1, 2, 3, 4, 5,   // -X
			6, 7, 8, 9, 10, 11,  // +X
			12, 13, 14, 15, 16, 17,  // -Y
			18, 19, 20, 21, 22, 23,  // +Y
			24, 25, 26, 27, 28, 29,  // -Z
			30, 31, 32, 33, 34, 35,  // +Z
		};
	}

	static std::vector<D3D11_INPUT_ELEMENT_DESC> CreateColorCubeInputLayout()
	{
		return
		{
			{"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Color", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}





	/////////////////////////////////////////
	// NormalCube
	/////////////////////////////////////////

	struct CubeVertexNorm
	{
		XrVector3f Position;
		XrVector3f Normal;
	};

	constexpr XrVector3f mX{ -1, 0, 0 };
	constexpr XrVector3f pX{ 1, 0, 0 };
	constexpr XrVector3f mY{ 0, -1, 0 };
	constexpr XrVector3f pY{ 0, 1, 0 };
	constexpr XrVector3f mZ{ 0, 0, -1 };
	constexpr XrVector3f pZ{ 0, 0, 1 };

	static std::vector<CubeVertexNorm> CreateNormalCubeVertices()
	{

		return std::vector<CubeVertexNorm>
		{
			CUBE_SIDE( LTB, LBF, LBB, LTB, LTF, LBF, mX )    // -X
			CUBE_SIDE( RTB, RBB, RBF, RTB, RBF, RTF, pX )        // +X
			CUBE_SIDE( LBB, LBF, RBF, LBB, RBF, RBB, mY )  // -Y
			CUBE_SIDE( LTB, RTB, RTF, LTB, RTF, LTF, pY )      // +Y
			CUBE_SIDE( LBB, RBB, RTB, LBB, RTB, LTB, mZ )   // -Z
			CUBE_SIDE( LBF, LTF, RTF, LBF, RTF, RBF, pZ )       // +Z
		};
	}

	static std::vector<unsigned short> CreateNormalCubeIndices()
	{
		return std::vector<unsigned short>
		{
			0, 1, 2, 3, 4, 5,   // -X
			6, 7, 8, 9, 10, 11,  // +X
			12, 13, 14, 15, 16, 17,  // -Y
			18, 19, 20, 21, 22, 23,  // +Y
			24, 25, 26, 27, 28, 29,  // -Z
			30, 31, 32, 33, 34, 35,  // +Z
		};
	}

	static std::vector<D3D11_INPUT_ELEMENT_DESC> CreateNormalCubeInputLayout()
	{
		return
		{
			{"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}

}