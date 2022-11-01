#pragma once

#include <vector>

namespace Katame
{
	class Graphics;
	class Drawable;
	class Mesh;
	class VCBuffer;
	class Model;

	class Renderer
	{
	public:
		Renderer( Graphics* gfx );
		~Renderer();
	public:
		void Submit( Drawable& drawable );
		void Submit( Mesh& mesh );
		void Submit( Model& model );
		void Execute( VCBuffer* modelCBuf );
		void Clear();
	private:
		Graphics* gfx;
		std::vector<Drawable*> m_Drawables;
		std::vector<Mesh*> m_Meshes;
		std::vector<Model*> m_Models;
	};
}