#pragma once

#include <vector>

#include "../Bindables/Buffer.h"
#include "../Bindables/InputLayout.h"
#include "../Bindables/VertexShader.h"
#include "../Bindables/PixelShader.h"
#include "../Bindables/Texture.h"
#include "../Bindables/Topology.h"
#include "../Bindables/Rasterizer.h"
#include "../Bindables/Blender.h"

#include <DirectXMath.h>

#include <openxr/openxr.h>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

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

		Mesh( const std::string& filename, Graphics* gfx );
		~Mesh();
	private:
		std::vector<Texture> loadMaterialTextures( aiMaterial* mat, aiTextureType type, std::string typeName, const aiScene* scene, Graphics* gfx );
		ID3D11ShaderResourceView* loadEmbeddedTexture( const aiTexture* embeddedTexture, Graphics* gfx );
	public:

		void Render( Graphics* gfx );
		void Update( float dt );
		void SetData( XrPosef pose, XrVector3f scale );

		DirectX::XMMATRIX GetModelMatrix();
		inline const std::string& GetFilePath() const { return m_FilePath; }
	private:
		bool flag = true;

		std::vector<Vertex> m_Vertices;
		std::vector<unsigned int> m_Indices;

		VertexBuffer* m_VertexBuffer;
		IndexBuffer* m_IndexBuffer;
		InputLayout* m_InputLayout;
		Texture* m_DiffTex;
		VertexShader* vs;
		PixelShader* ps;
		Topology* m_Topology;
		Rasterizer* m_Rasterizer;
		Blender* m_Blender;
		XrPosef Pose = { { 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
		XrVector3f Scale = { 0.025f, 0.025f, 0.025f };
		std::string m_FilePath;
	};
}