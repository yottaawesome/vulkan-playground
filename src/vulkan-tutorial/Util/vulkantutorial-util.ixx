export module vulkantutorial:util;

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
