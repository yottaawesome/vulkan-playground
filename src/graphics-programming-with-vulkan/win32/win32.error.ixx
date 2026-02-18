export module vulkangfx:win32.error;
import std;
import :win32.exports;

namespace Win32
{
	auto ErrorCodeToAnsi(Win32::DWORD errorCode) -> std::string
	{
		if (errorCode == 0)
			return {};
		auto messageBuffer = static_cast<char*>(nullptr);
		const auto size = FormatMessageA(
			FormatMessageFlags::AllocateBuffer | FormatMessageFlags::FromSystem | FormatMessageFlags::IgnoreInserts,
			nullptr,
			errorCode,
			0,
			reinterpret_cast<LPSTR>(&messageBuffer),
			0,
			nullptr
		);
		auto message = std::string(messageBuffer, size);
		LocalFree(messageBuffer);
		return message;
	}

	auto ErrorCodeToUnicode(Win32::DWORD errorCode) -> std::wstring
	{
		if (errorCode == 0)
			return {};
		auto messageBuffer = static_cast<wchar_t*>(nullptr);
		const auto size = FormatMessageW(
			FormatMessageFlags::AllocateBuffer | FormatMessageFlags::FromSystem | FormatMessageFlags::IgnoreInserts,
			nullptr,
			errorCode,
			0,
			reinterpret_cast<LPWSTR>(&messageBuffer),
			0,
			nullptr
		);
		auto message = std::wstring(messageBuffer, size);
		LocalFree(messageBuffer);
		return message;
	}

	struct Error : std::runtime_error
	{
		DWORD Code = 0;
		Error() noexcept = default;
		explicit Error(DWORD code) noexcept
			: std::runtime_error{ ErrorCodeToAnsi(code) }
		{}
	};
}