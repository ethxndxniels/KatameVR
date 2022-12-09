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
		Material() = default;
		Material( Graphics& gfx, const aiMaterial& material, const std::filesystem::path& path ) noexcept;
		~Material();
		void Bind( Graphics& gfx );
	private:
		InputLayout* m_InputLayout = nullptr;
		Texture* m_DiffTex = nullptr;
		Texture* m_SpecTex = nullptr;
		Texture* m_NormTex = nullptr;
		VertexShader* m_VS = nullptr;
		PixelShader* m_PS = nullptr;
		Topology* m_Topology = nullptr;
		Rasterizer* m_Rasterizer = nullptr;
		Blender* m_Blender = nullptr;
		Sampler* m_Sampler = nullptr;
	private:
		std::string modelPath;
		std::string name;
	};
}
