module;

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

export module vulkangfx:win32.exports;

export namespace Win32
{
	using
		::HINSTANCE,
		::LPWSTR,
		::UINT,
		::HANDLE,
		::FreeLibrary,
		::CloseHandle
		;
}
