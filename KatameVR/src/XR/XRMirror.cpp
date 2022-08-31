#include "XRMirror.h"

#include "../Core/Log.h"

namespace Katame
{
	XRMirror::XRMirror( int nWidth, int nHeight, const char* pTitle, const char* sLogFile )
	{
		// Setup helper utilities class
		//m_pUtils = new Utils( "XRMirror", sLogFile );

		// Initialize glfw
		//glfwInit();
		//glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
		//glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
		//glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
		//// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

		//// Create glfw window
		//m_Mirror = glfwCreateWindow( nWidth, nHeight, pTitle, NULL, NULL );

		//if (m_Mirror == NULL)
		//{
		//	std::string eMessage = "Failed to create GLFW window";
		//	KM_CORE_ERROR( "{}", eMessage );
		//	glfwTerminate();
		//	throw eMessage;
		//}

		//glfwMakeContextCurrent( m_Mirror );
		//KM_CORE_INFO( "Window for OpenXR Mirror context created" );

		//// Check glad can be loaded before we use any OpenGL calls
		///*if (!gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress ))
		//{
		//	std::string eMessage = "Failed to initialize GLAD";
		//	KM_CORE_ERROR( "{}", eMessage );
		//	glfwTerminate();
		//	throw eMessage;
		//}*/

		//// Create mirror
		////glViewport( 0, 0, nWidth, nHeight );
		//KM_CORE_INFO( "Mirror created {}x{}", nWidth, nHeight );

		//// Set callback for window resizing
		//glfwSetFramebufferSizeCallback( m_Mirror, []( GLFWwindow* mirror, int width, int height ) { glViewport( 0, 0, width, height ); } );
	}

	XRMirror::~XRMirror()
	{
		//delete m_pUtils;
		//glfwDestroyWindow( GetWindow() );
		//glfwTerminate();
	}


	//unsigned int XRMirror::LoadTexture( const wchar_t* pTextureFile, GLuint nShader, const char* pSamplerParam, GLint nMinFilter, GLint nMagnitudeFilter, GLint nWrapS, GLint nWrapT )
	//{
	//	unsigned int nTexture = 0;

	//	glGenTextures( 1, &nTexture );
	//	glBindTexture( GL_TEXTURE_2D, nTexture );

	//	// Set the texture parameters
	//	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	//	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	//	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	//	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	//	// Load image from disk
	//	int nWidth, nHeight, nChannels;
	//	char pTexture[MAX_PATH] = "";
	//	std::wcstombs( pTexture, pTextureFile, MAX_PATH );

	//	stbi_set_flip_vertically_on_load( true );

	//	unsigned char* textureData = stbi_load( pTexture, &nWidth, &nHeight, &nChannels, 0 );
	//	if (textureData)
	//		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, nWidth, nHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData );
	//	else
	//		KM_CORE_WARN( "Unable to load cube texture from disk ({})", pTexture );

	//	stbi_image_free( textureData );

	//	// Set the 2d texture sampler param in the shader
	//	glUseProgram( nShader );
	//	glUniform1i( glGetUniformLocation( nShader, pSamplerParam ), 0 );

	//	return nTexture;
	//}
}