#include "Win32Window.h"

#include "../Core/Log.h"

#include "../../vendor/imgui/backends/imgui_impl_win32.h"

namespace Katame
{
	Win32Window::Win32Window( int nWidth, int nHeight, const char* name )
		: u_Width( nWidth ), u_Height( nHeight ),
		  m_hInst( GetModuleHandle( nullptr ) )
	{
		// register window class
		WNDCLASSEX  wc = { 0 };
		wc.cbSize = sizeof( wc );
		wc.style = CS_OWNDC;
		wc.lpfnWndProc = HandleMsgSetup;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = m_hInst;
		wc.hCursor = nullptr;
		wc.hbrBackground = nullptr;
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = (LPCWSTR)name;
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
			(LPCWSTR)name, (LPCWSTR)name,
			WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
			CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
			nullptr, nullptr, m_hInst, this
		);
		// check for error
		if (m_hWnd == nullptr)
		{
			KM_CORE_ERROR( "Failed to create window" );
			throw std::exception();
		}

		// newly created windows start off as hidden
		ShowWindow( m_hWnd, SW_SHOWDEFAULT );

		ImGui_ImplWin32_Init( m_hWnd );
	}

	Win32Window::~Win32Window()
	{
		DestroyWindow( m_hWnd );
	}

	std::optional<int> Win32Window::ProcessMessages() noexcept
	{
		MSG msg;
		while (PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ))
		{
			if (msg.message == WM_QUIT)
				return (int)msg.wParam;

			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}

		// return empty optional when not quitting app
		return {};
	}

	unsigned int Win32Window::LoadTexture( const wchar_t* pTextureFile, const char* pSamplerParam )
	{
		return {};
		//unsigned int nTexture = 0;

		////glGenTextures( 1, &nTexture );
		////glBindTexture( GL_TEXTURE_2D, nTexture );

		//// Set the texture parameters
		////glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		////glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		////glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		////glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

		//// Load image from disk
		//int nWidth, nHeight, nChannels;
		//char pTexture[MAX_PATH] = "";
		//std::wcstombs( pTexture, pTextureFile, MAX_PATH );

		//stbi_set_flip_vertically_on_load( true );

		//unsigned char* textureData = stbi_load( pTexture, &nWidth, &nHeight, &nChannels, 0 );
		////if (textureData)
		////	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, nWidth, nHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData );
		////else
		////	KM_CORE_WARN( "Unable to load cube texture from disk ({})", pTexture );

		//stbi_image_free( textureData );

		//// Set the 2d texture sampler param in the shader
		////glUseProgram( nShader );
		////glUniform1i( glGetUniformLocation( nShader, pSamplerParam ), 0 );

		//return nTexture;
	}

	LRESULT Win32Window::HandleMsgSetup( HWND m_hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) noexcept
	{
		// use create parameter passed in from CreateWindow() to store window class pointer
		if (msg == WM_NCCREATE)
		{
			// extract ptr to window class from creation data
			const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
			Win32Window* const pWnd = static_cast<Win32Window*>(pCreate->lpCreateParams);

			// set WinApi-managed user data to store ptr to window class
			SetWindowLongPtr( m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd) );

			// set message proc to normal (non-setup) handler now that setup is finished
			SetWindowLongPtr( m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Win32Window::HandleMsgThunk) );

			// forward message to window class handler
			return pWnd->HandleMsg( m_hWnd, msg, wParam, lParam );
		}
		// if we get a message before the WM_NCCREATE message, handle with default handler;
		return DefWindowProc( m_hWnd, msg, wParam, lParam );
	}

	LRESULT WINAPI Win32Window::HandleMsgThunk( HWND m_hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) noexcept
	{
		// retrieve ptr to window class
		Win32Window* const pWnd = reinterpret_cast<Win32Window*>(GetWindowLongPtr( m_hWnd, GWLP_USERDATA ));

		// forward message to window class handler
		return pWnd->HandleMsg( m_hWnd, msg, wParam, lParam );
	}

	LRESULT Win32Window::HandleMsg( HWND m_hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) noexcept
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