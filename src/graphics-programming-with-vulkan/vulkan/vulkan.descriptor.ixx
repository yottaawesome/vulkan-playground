export module vulkangfx:vulkan.descriptors;
import std;
import :error;
import :vulkan.exports;
import :vulkan.error;

export namespace Vulkan
{
	struct DescriptorSetLayoutDeleter
	{
		vkr::VkDevice Device = nullptr;
		DescriptorSetLayoutDeleter() = default;
		DescriptorSetLayoutDeleter(vkr::VkDevice device)
			: Device(device)
		{
			if (not Device)
				throw Error::RuntimeError("DescriptorSetLayoutDeleter requires a valid VkDevice.");
		}
		void operator()(vkr::VkDescriptorSetLayout layout) const noexcept
		{
			vkr::vkDestroyDescriptorSetLayout(Device, layout, nullptr);
		}
	};
	using DescriptorSetLayoutUniquePtr = std::unique_ptr<std::remove_pointer_t<vkr::VkDescriptorSetLayout>, DescriptorSetLayoutDeleter>;

	class DescriptorSetLayout
	{
	public:
		DescriptorSetLayout(vkr::VkDevice device, std::ranges::range auto&& layoutBindings)
		{
			auto layoutInfo = vkr::VkDescriptorSetLayoutCreateInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.bindingCount = static_cast<std::uint32_t>(layoutBindings.size()),
				.pBindings = layoutBindings.data()
			};

			// https://docs.vulkan.org/refpages/latest/refpages/source/vkCreateDescriptorSetLayout.html
			auto raw = vkr::VkDescriptorSetLayout{};
			auto result = Result{
				vkr::vkCreateDescriptorSetLayout(
					device,
					&layoutInfo,
					nullptr,
					&raw
				) };
			if (not result)
				throw Vulkan::VulkanError{ result, "Failed to create descriptor set layout." };
			layout = DescriptorSetLayoutUniquePtr{ raw, DescriptorSetLayoutDeleter{ device } };
		}

		DescriptorSetLayout(DescriptorSetLayoutUniquePtr layoutIn)
			: layout(std::move(layoutIn))
		{}

		constexpr auto GetHandle(this auto&& self) noexcept -> vkr::VkDescriptorSetLayout
		{
			return self.layout.get();
		}
	private:
		DescriptorSetLayoutUniquePtr layout;
	};




	struct DescriptorPoolDeleter
	{
		vkr::VkDevice Device = nullptr;
		DescriptorPoolDeleter() = default;
		DescriptorPoolDeleter(vkr::VkDevice device)
			: Device(device)
		{
			if (not Device)
				throw Error::RuntimeError("DescriptorPoolDeleter requires a valid VkDevice.");
		}
		void operator()(vkr::VkDescriptorPool pool) const noexcept
		{
			vkr::vkDestroyDescriptorPool(Device, pool, nullptr);
		}
	};
	using DescriptorPoolUniquePtr = std::unique_ptr<std::remove_pointer_t<vkr::VkDescriptorPool>, DescriptorPoolDeleter>;

	class DescriptorPool
	{
	public:
		DescriptorPool(
			std::ranges::range auto&& poolSizes, 
			std::uint32_t maxSets, 
			vkr::VkDevice device,
			vkr::VkDescriptorPoolCreateFlags flags
		)
		{
			auto poolInfo = vkr::VkDescriptorPoolCreateInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
				.flags = flags,
				.maxSets = maxSets,
				.poolSizeCount = static_cast<std::uint32_t>(poolSizes.size()),
				.pPoolSizes = poolSizes.data(),
			};
			auto raw = vkr::VkDescriptorPool{};
			auto result = Result{
				vkr::vkCreateDescriptorPool(
					device,
					&poolInfo,
					nullptr,
					&raw
				)};
			if (not result)
				throw VulkanError{ result, "Failed to create descriptor pool." };
			pool = DescriptorPoolUniquePtr{ raw, DescriptorPoolDeleter{ device } };
		}

		constexpr auto GetHandle(this const DescriptorPool& self) noexcept -> vkr::VkDescriptorPool
		{
			return self.pool.get();
		}
	private:
		DescriptorPoolUniquePtr pool;
	};




	struct DescriptorSetDeleter
	{
		vkr::VkDevice Device = nullptr;
		vkr::VkDescriptorPool Pool = nullptr;
		DescriptorSetDeleter() = default;
		DescriptorSetDeleter(vkr::VkDevice device, vkr::VkDescriptorPool pool)
			: Device(device), Pool(pool)
		{
			if (not Device)
				throw Error::RuntimeError("DescriptorSetDeleter requires a valid VkDevice.");
			if (not Pool)
				throw Error::RuntimeError("DescriptorSetDeleter requires a valid VkDescriptorPool.");
		}
		void operator()(vkr::VkDescriptorSet descriptorSet) const noexcept
		{
			vkr::vkFreeDescriptorSets(Device, Pool, 1, &descriptorSet);
		}
	};
	using DescriptorSetUniquePtr = std::unique_ptr<std::remove_pointer_t<vkr::VkDescriptorSet>, DescriptorSetDeleter>;

	class DescriptorSet
	{
	public:
		static auto Create(
			vkr::VkDevice device,
			vkr::VkDescriptorPool pool,
			vkr::VkDescriptorSetLayout layout
		) -> DescriptorSetUniquePtr
		{
			auto allocInfo = vkr::VkDescriptorSetAllocateInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
				.descriptorPool = pool,
				.descriptorSetCount = 1,
				.pSetLayouts = &layout
			};
			auto set = vkr::VkDescriptorSet{};
			auto result = Result{
				vkr::vkAllocateDescriptorSets(
					device,
					&allocInfo,
					&set
				)};
			if (not result)
				throw VulkanError{ result, "Failed to allocate descriptor set." };
			return DescriptorSetUniquePtr{ set, DescriptorSetDeleter{ device, pool } };
		}

		DescriptorSet(DescriptorSetUniquePtr setIn) 
			: set(std::move(setIn))
		{ }

		constexpr auto GetHandle(this auto&& self) noexcept -> vkr::VkDescriptorSet
		{
			return self.set.get();
		}

		void Free(this auto&& self) // return to pool
		{
			self.set.reset();
		}

	private:
		DescriptorSetUniquePtr set;
	};
}
