export module vulkantutorial:util;

export namespace VulkanTutorial
{
	constexpr bool IsDebug = 
		[] static consteval noexcept -> bool
		{
#ifdef _DEBUG	
			return true;
#endif
			return false;
		}();
	constexpr bool IsRelease = not IsDebug;
	constexpr bool EnableValidationLayers = IsDebug;
}
