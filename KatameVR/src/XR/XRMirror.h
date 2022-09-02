#pragma once

#include <map>

#include "XRRender.h"
#include <stb_image.h>

#include "../Utilities/WinDefines.h"

namespace Katame
{
	class XRMirror
	{
	public:
		XRMirror( int nWidth, int nHeight, const char* pTitle );
		~XRMirror();
	public:
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