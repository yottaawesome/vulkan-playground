export module vulkangfx:file.file;
import std;
import :error;

export namespace File
{
	template<int VMode>
	struct File
	{
		std::string_view FileName = "somefile.txt";
		std::unique_ptr<std::ofstream> File;

		auto GetLogFile(this File& self) -> std::ofstream&
		{
			if (self.File)
				return *self.File;
			self.File = std::make_unique<std::ofstream>(std::string{ self.FileName }, VMode);
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

	using OutputFile = File<std::ios::out>;
	using AppendedOutputFile = File<std::ios::app>;
	using InputFile = File<std::ios::in>;
	using InputBinaryFile = File<std::ios::in | std::ios::binary>;
	using OutputBinaryFile = File<std::ios::out | std::ios::binary>;
	using InputOutputBinaryFile = File<std::ios::in | std::ios::out | std::ios::binary>;
}
