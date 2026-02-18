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
		if (size == 0)
			return std::format("Unknown error code {}", errorCode);
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
		if (size == 0)
			return std::format(L"Unknown error code {}", errorCode);
		auto message = std::wstring(messageBuffer, size);
		LocalFree(messageBuffer);
		return message;
	}

	struct Win32ErrorCode
	{
		const DWORD Code = 0;
		auto ToString(this const Win32ErrorCode& self) -> std::string
		{
			return ErrorCodeToAnsi(self.Code);
		}
	};

	struct Error : std::runtime_error
	{
		const DWORD Code = 0;

		explicit Error(
			DWORD code, 
			std::string_view message = "An exception occurred",
			const std::source_location& loc = std::source_location::current()
		) noexcept
			: std::runtime_error{ Format(code, message, loc) }
			, Code(code)
		{}

		static auto Format(
			DWORD code, 
			std::string_view message, 
			const std::source_location& loc
		) -> std::string
		{
			return std::format(
				"{} {} (code {}) at {}:{}:{}", 
				message,
				ErrorCodeToAnsi(code), 
				code, 
				loc.file_name(), 
				loc.line(), 
				loc.column()
			);
		}
	};
}