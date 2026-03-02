export module vulkangfx:file;
import std;

export namespace Vulkan
{
	struct OutputFile
	{
		std::string_view FileName = "somefile.txt";
		int Mode = std::ios::app;
		std::unique_ptr<std::ofstream> File;

		auto GetLogFile(this OutputFile& self) -> std::ofstream&
		{
			if (self.File)
				return *self.File;
			self.File = std::make_unique<std::ofstream>(std::string{ self.FileName }, self.Mode);
			return *self.File;
		}

		auto operator<<(this auto&& self, std::string_view message) -> decltype(self)
		{
			auto& logFile = self.GetLogFile();
			if (logFile)
				logFile << message << std::endl;
			return std::forward<decltype(self)>(self);
		}

		auto operator<<(this auto&& self, std::ostream& (*manip)(std::ostream&)) -> decltype(self)
		{
			auto& logFile = self.GetLogFile();
			if (logFile)
				manip(logFile);
			return std::forward<decltype(self)>(self);
		}
	};
}
