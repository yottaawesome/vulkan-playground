module;

#include <stdio.h>
#include <stdlib.h>

export module vulkangfx:logging;
import std;
import :win32;
import :error;
import :string;

namespace Vulkan::Log
{
	enum class LoggingMode
	{
		Console,
		File,
	};

	constexpr auto CurrentLoggingMode = LoggingMode::Console;
	struct ConsoleInit
	{
		~ConsoleInit() noexcept
		{
			if (CurrentLoggingMode == LoggingMode::Console)
				Win32::FreeConsole();
		}

		ConsoleInit() noexcept
		{
			if (CurrentLoggingMode != LoggingMode::Console)
				return;
			Win32::AllocConsole();
			auto fDummy = static_cast<FILE*>(nullptr);
			freopen_s(&fDummy, "CONIN$", "r", stdin);
			freopen_s(&fDummy, "CONOUT$", "w", stderr);
			freopen_s(&fDummy, "CONOUT$", "w", stdout);
		}
	} const ConsoleInit;

	struct LogFileHandler
	{
		std::string_view FileName = "log.txt";
		std::unique_ptr<std::ofstream> of;

		auto GetLogFile(this LogFileHandler& self) -> std::ofstream&
		{
			if (self.of)
				return *self.of;
			self.of = std::make_unique<std::ofstream>(std::string{ self.FileName }, std::ios::app);
			return *self.of;
		}

		auto operator<<(this auto&& self, std::string_view message) -> decltype(self)
		{
			auto& logFile = self.GetLogFile();
			if (logFile)
				logFile << message << std::endl;
			return std::forward<decltype(self)>(self);
		}

		auto operator<<(this auto&& self, std::ostream&(*manip)(std::ostream&)) -> decltype(self)
		{
			auto& logFile = self.GetLogFile();
			if (logFile)
				manip(logFile);
			return std::forward<decltype(self)>(self);
		}
	} constexpr LogFile{ "log.txt" };

	template<LoggingMode T = CurrentLoggingMode>
	struct InternalLogger
	{
		static void Info(std::string_view source, std::string_view message)
		{
			auto formatted = std::format(
				"[{}] [info] [{}]: {}\n",
				std::chrono::zoned_time{ std::chrono::current_zone(), std::chrono::system_clock::now() },
				source,
				message
			);

			if constexpr (T == LoggingMode::Console)
			{
				std::println("{}", formatted);
			}
			else if constexpr (T == LoggingMode::File)
			{
				LogFile << formatted;
			}
		}

		static void Warning(std::string_view source, std::string_view message)
		{
			auto formatted = std::format(
				"[{}] [warning] [{}]: {}\n",
				std::chrono::zoned_time{ std::chrono::current_zone(), std::chrono::system_clock::now() },
				source,
				message
			);

			if constexpr (T == LoggingMode::Console)
			{
				std::println("{}", formatted);
			}
			else if constexpr (T == LoggingMode::File)
			{
				LogFile << formatted;
			}
		}

		static void Error(std::string_view source, std::string_view message)
		{
			auto formatted = std::format(
				"[{}] [error] [{}]: {}\n",
				std::chrono::zoned_time{ std::chrono::current_zone(), std::chrono::system_clock::now() },
				source,
				message
			);

			if constexpr (T == LoggingMode::Console)
			{
				std::println(std::cerr, "{}", formatted);
			}
			else if constexpr (T == LoggingMode::File)
			{
				LogFile << formatted;
			}
		}
	};
}

export namespace Vulkan::Log
{
	template<FixedString Name>
	struct Logger
	{
		template<typename... TArgs>
		void Info(std::format_string<TArgs...> fmt, TArgs&&...args)
		{
			InternalLogger<>::Info(Name.ToView(), std::format(fmt, std::forward<TArgs>(args)...));
		}
		template<typename... TArgs>
		void Warning(std::format_string<TArgs...> fmt, TArgs&&...args)
		{
			InternalLogger<>::Warning(Name.ToView(), std::format(fmt, std::forward<TArgs>(args)...));
		}
		template<typename... TArgs>
		void Error(std::format_string<TArgs...> fmt, TArgs&&...args)
		{
			InternalLogger<>::Error(Name.ToView(), std::format(fmt, std::forward<TArgs>(args)...));
		}
	};
}
