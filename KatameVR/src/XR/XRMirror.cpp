#include "XRMirror.h"

#include "../Core/Log.h"

namespace Katame
{
	XRMirror::XRMirror( int nWidth, int nHeight, const char* pTitle )
		: u_Width( nWidth ), u_Height( nHeight )
	{
		// Initialize glfw
		//glfwInit();
		//glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
		//glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
		//glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
		//// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		// register window class
		WNDCLASSEX  wc = { 0 };
		//wc.cbSize = sizeof( wc );
		wc.cbSize = sizeof( wc );
		wc.style = CS_OWNDC;
		wc.lpfnWndProc = HandleMsgSetup;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = m_hInst;
		wc.hCursor = nullptr;
		wc.hbrBackground = nullptr;
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = (LPCWSTR)pTitle;
		RegisterClassEx( &wc );

		// create window size based on desired client region size
		RECT wr;
		wr.left = 100;
		wr.right = nWidth + wr.left;
		wr.top = 100;
		wr.bottom = nHeight + wr.top;
		if (AdjustWindowRect( &wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE ) == 0)
			throw std::exception();

		// create window and get hWnd
		m_hWnd = CreateWindow(
			(LPCWSTR)pTitle, (LPCWSTR)pTitle,
			WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
			CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
			nullptr, nullptr, m_hInst, this
		);
		// check for error
		if (m_hWnd == nullptr)
		{
			throw std::exception();
		}

		// newly created windows start off as hidden
		ShowWindow( m_hWnd, SW_SHOWDEFAULT );

		
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

	LRESULT XRMirror::HandleMsgSetup( HWND m_hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) noexcept
	{
		// use create parameter passed in from CreateWindow() to store window class pointer
		if (msg == WM_NCCREATE)
		{
			// extract ptr to window class from creation data
			const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
			XRMirror* const pWnd = static_cast<XRMirror*>(pCreate->lpCreateParams);

			// set WinApi-managed user data to store ptr to window class
			SetWindowLongPtr( m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd) );

			// set message proc to normal (non-setup) handler now that setup is finished
			SetWindowLongPtr( m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&XRMirror::HandleMsgThunk) );

			// forward message to window class handler
			return pWnd->HandleMsg( m_hWnd, msg, wParam, lParam );
		}
		// if we get a message before the WM_NCCREATE message, handle with default handler;
		return DefWindowProc( m_hWnd, msg, wParam, lParam );
	}

	LRESULT WINAPI XRMirror::HandleMsgThunk( HWND m_hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) noexcept
	{
		// retrieve ptr to window class
		XRMirror* const pWnd = reinterpret_cast<XRMirror*>(GetWindowLongPtr( m_hWnd, GWLP_USERDATA ));

		// forward message to window class handler
		return pWnd->HandleMsg( m_hWnd, msg, wParam, lParam );
	}

	extern LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

	LRESULT XRMirror::HandleMsg( HWND m_hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) noexcept
	{
		switch (msg)
		{
		case WM_CLOSE:
			PostQuitMessage( 0 );
			return 0;
		}
		return DefWindowProc( m_hWnd, msg, wParam, lParam );
	}
}