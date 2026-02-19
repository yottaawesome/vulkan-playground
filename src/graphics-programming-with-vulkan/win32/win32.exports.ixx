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
		::CloseHandle,
		::MessageBoxA
		;

	namespace EventAccess
	{
		constexpr auto AllAccess = EVENT_ALL_ACCESS;
		constexpr auto ModifyState = EVENT_MODIFY_STATE;
		constexpr auto Synchronize = SYNCHRONIZE;
	}

	namespace MessageBoxFlags
	{
		enum : UINT
		{
			Ok = MB_OK,
			OkCancel = MB_OKCANCEL,
			AbortRetryIgnore = MB_ABORTRETRYIGNORE,
			YesNoCancel = MB_YESNOCANCEL,
			YesNo = MB_YESNO,
			RetryCancel = MB_RETRYCANCEL,
			Critical = MB_ICONERROR,
			Question = MB_ICONQUESTION,
			Warning = MB_ICONWARNING,
			Information = MB_ICONINFORMATION
		};
	}

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
