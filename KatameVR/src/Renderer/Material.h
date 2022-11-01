#pragma once

#include "../Graphics/Graphics.h"

// Bindables
#include "../Bindables/Buffer.h"
#include "../Bindables/InputLayout.h"
#include "../Bindables/VertexShader.h"
#include "../Bindables/PixelShader.h"
#include "../Bindables/Texture.h"
#include "../Bindables/Topology.h"
#include "../Bindables/Rasterizer.h"
#include "../Bindables/Blender.h"
#include "../Bindables/Sampler.h"

#include <vector>
#include <filesystem>

struct aiMaterial;
struct aiMesh;

namespace Katame
{
	class VertexBuffer;
	class IndexBuffer;

	class Material
	{
	public:
		Material( Graphics* gfx, const aiMaterial& material, const std::filesystem::path& path ) noexcept;
	private:
		InputLayout* m_InputLayout;
		Texture* m_DiffTex;
		Texture* m_SpecTex;
		Texture* m_NormTex;
		VertexShader* m_VS;
		PixelShader* m_PS;
		Topology* m_Topology;
		Rasterizer* m_Rasterizer;
		Blender* m_Blender;
		Sampler* m_Sampler;
	private:
		std::string modelPath;
		std::string name;
	};
}
