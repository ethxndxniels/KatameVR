#pragma once

#include <map>

#include "XRRender.h"

#include <stb_image.h>

namespace Katame
{
	class XRMirror
	{
	public:
		XRMirror( int nWidth, int nHeight, const char* pTitle, const char* sLogFile );
		~XRMirror();
		/*GLFWwindow* GetWindow() { return m_Mirror; }
		unsigned int LoadTexture(
			const wchar_t* pTextureFile,
			GLuint nShader,
			const char* pSamplerParam,
			GLint nMinFilter = GL_LINEAR,
			GLint nMagnitudeFilter = GL_NEAREST,
			GLint nWrapS = GL_REPEAT,
			GLint nWrapT = GL_REPEAT );*/
	private:
	//	GLFWwindow* m_Mirror;
	};
}