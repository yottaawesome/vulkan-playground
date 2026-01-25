export module vulkantutorial:util_build;

export namespace VulkanTutorial::Util
{
	constexpr bool IsDebug =
#ifdef _DEBUG	
		true;
#else
		false;
#endif
	constexpr bool IsRelease = not IsDebug;
	constexpr bool EnableValidationLayers = IsDebug;
}
