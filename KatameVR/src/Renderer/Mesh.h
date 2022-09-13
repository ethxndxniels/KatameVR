#pragma once

#include <vector>

#include "../Bindables/Buffer.h"
#include "../Bindables/InputLayout.h"
#include "../Bindables/VertexShader.h"
#include "../Bindables/PixelShader.h"

#include <DirectXMath.h>

namespace Katame {

	class Graphics;

	class Mesh
	{
	public:
		struct Vertex
		{
			DirectX::XMFLOAT3 Position;
			DirectX::XMFLOAT3 Normal;
			DirectX::XMFLOAT3 Tangent;
			DirectX::XMFLOAT3 Binormal;
			DirectX::XMFLOAT2 Texcoord;
		};
		static_assert(sizeof( Vertex ) == 14 * sizeof( float ));
		static const int NumAttributes = 5;

		struct Index
		{
			uint32_t V1, V2, V3;
		};
		static_assert(sizeof( Index ) == 3 * sizeof( uint32_t ));

		Mesh( const std::string& filename, Graphics* gfx );
		~Mesh();

		void Render( Graphics* gfx );
		DirectX::XMMATRIX GetModelMatrix();
		inline const std::string& GetFilePath() const { return m_FilePath; }
	private:
		std::vector<Vertex> m_Vertices;
		std::vector<Index> m_Indices;

		VertexBuffer* m_VertexBuffer;
		IndexBuffer* m_IndexBuffer;
		InputLayout* m_InputLayout;
		VertexShader* vs;
		PixelShader* ps;

		std::string m_FilePath;
	};
}