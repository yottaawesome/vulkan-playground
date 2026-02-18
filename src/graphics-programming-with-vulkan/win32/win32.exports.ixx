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
		::SECURITY_DESCRIPTOR,
		::SECURITY_ATTRIBUTES,
		::DWORD,
		::HRESULT,
		::LocalFree,
		::GetLastError,
		::FormatMessageA,
		::FormatMessageW,
		::CreateEventW,
		::ResetEvent,
		::SetEvent,
		::OpenEventW,
		::FreeLibrary,
		::CloseHandle
		;

	namespace FormatMessageFlags
	{
		enum : DWORD
		{
			AllocateBuffer = FORMAT_MESSAGE_ALLOCATE_BUFFER,
			FromSystem = FORMAT_MESSAGE_FROM_SYSTEM,
			IgnoreInserts = FORMAT_MESSAGE_IGNORE_INSERTS
		};
	}
}
