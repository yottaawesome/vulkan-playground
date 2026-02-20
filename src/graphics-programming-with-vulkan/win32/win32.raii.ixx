export module vulkangfx:win32.raii;
import std;
import :raii;
import :win32.exports;

export namespace Win32
{
	using HInstanceUniquePtr = Raii::DirectUniquePtr<HINSTANCE, FreeLibrary>;
	using HandleUniquePtr = Raii::IndirectUniquePtr<HANDLE, CloseHandle>;
}