export module volkus:vkx.physicaldevicefeatures;
import vulkanlib;

export namespace Volkus::vkx
{
	struct PhysicalDeviceFeatures
	{
		PhysicalDeviceFeatures(VkPhysicalDevice device)
		{
			vkGetPhysicalDeviceFeatures2(device, &f10);
		}

		VkPhysicalDeviceVulkan14Features f14{ 
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES 
		};
		VkPhysicalDeviceVulkan13Features f13{ 
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, 
			.pNext = &f14
		};
		VkPhysicalDeviceVulkan12Features f12{ 
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, 
			.pNext = &f13
		};
		VkPhysicalDeviceVulkan11Features f11{ 
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES, 
			.pNext = &f12
		};
		VkPhysicalDeviceFeatures2 f10{ 
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,          
			.pNext = &f11
		};
	};
}