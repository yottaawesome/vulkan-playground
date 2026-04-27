module;

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

export module volkus:win32;

export namespace Volkus::Win32
{
	using 
		::HINSTANCE,
		::HWND,
		::LRESULT,
		::WPARAM,
		::LPARAM,
		::LPWSTR,
		::UINT
		;
}
