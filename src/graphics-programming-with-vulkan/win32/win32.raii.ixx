export module vulkangfx:win32.raii;
import std;
import :raii;
import :win32.exports;

namespace Win32
{
	using HInstanceUniquePtr = Raii::DirectUniquePtr<HINSTANCE, FreeLibrary>;
	using HandleUniquePtr = Raii::DirectUniquePtr<HANDLE, CloseHandle>;
}