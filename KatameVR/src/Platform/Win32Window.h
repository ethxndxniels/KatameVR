#pragma once

#include <map>

#include <stb_image.h>

#include "../Utilities/WinDefines.h"

#include <optional>

namespace Katame
{
	class Win32Window
	{
	public:
		Win32Window( int width, int height, const char* name );
		~Win32Window();
	public:
		std::optional<int> ProcessMessages() noexcept;
		unsigned int LoadTexture( const wchar_t* pTextureFile, const char* pSamplerParam );
		static LRESULT CALLBACK HandleMsgSetup( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) noexcept;
		static LRESULT WINAPI HandleMsgThunk( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) noexcept;
		LRESULT HandleMsg( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) noexcept;
	private:
		HWND m_hWnd;
		unsigned int u_Width;
		unsigned int u_Height;
		HINSTANCE m_hInst;
	};
}
