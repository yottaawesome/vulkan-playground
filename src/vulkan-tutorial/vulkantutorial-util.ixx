export module vulkantutorial:util;

export namespace VulkanTutorial
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
