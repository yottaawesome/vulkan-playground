import volkus;
import modelviewer;

auto wWinMain(
	Volkus::Win32::HINSTANCE, Volkus::Win32::HINSTANCE, Volkus::Win32::LPWSTR, int
) -> int
{
	Volkus::Win32::Crt::SetAbortBehavior(
		Volkus::Win32::Crt::CallReportFault,
		Volkus::Win32::Crt::CallReportFault | Volkus::Win32::Crt::WriteAbortMsg
	);

	auto instance = ModelViewer::InstanceFactory{}(true);
	auto debugMessenger = ModelViewer::DebugMessengerFactory{}(instance.Get());
	auto physicalDevice = ModelViewer::SelectPhysicalDevice(instance);
	auto queueIndex = ModelViewer::FindQueueIndex(physicalDevice);
	auto device = ModelViewer::CreateDevice(queueIndex, physicalDevice);

	return 0;
}
