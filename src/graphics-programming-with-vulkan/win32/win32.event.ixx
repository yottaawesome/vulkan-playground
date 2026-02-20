export module vulkangfx:win32.event;
import std;
import :win32.exports;
import :win32.raii;
import :win32.error;

export namespace Win32
{
	struct CreateEventFactory
	{
		std::optional<SECURITY_ATTRIBUTES> SecurityAttributes;
		std::wstring Name;
		bool ManualReset = false;
		bool InitialState = false;

		auto CreateEvent() -> HandleUniquePtr
		{
			auto handle = Win32::HANDLE{
				CreateEventW(
					//Can also use std::to_address(SecurityAttributes.operator->())
					SecurityAttributes.has_value() ? &*SecurityAttributes : nullptr,
					ManualReset,
					InitialState,
					Name.empty() ? nullptr : Name.c_str()
				) };
			if (not handle)
			{
				throw Error{ GetLastError() };
			}
			return HandleUniquePtr{ handle };
		}
	};
}
