module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

export module vulkan26:win32;

export namespace Win32
{
	using 
		::HINSTANCE,
		::LPWSTR,
		::HANDLE,
		::DWORD,
		::HWND,
		::GetLastError
		;
}
