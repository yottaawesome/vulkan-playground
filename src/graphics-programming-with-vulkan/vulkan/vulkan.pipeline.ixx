/* An AI generated summary of what a Vulkan pipeline is.
* 
* A Vulkan pipeline is a pre-compiled GPU state object that defines the entire processing path for a draw or dispatch
* call. It bundles together everything the GPU needs so there's no runtime state assembly.
*
* Two main types:
*  - Graphics pipeline — vertex input, shaders (vertex/fragment/etc.), rasterization, depth/stencil, blending,
* viewport, multisampling, render pass format
*  - Compute pipeline — just a compute shader + pipeline layout
*
* Key concepts:
*  - Immutable by design — all state is baked at creation time, so the driver can optimize aggressively. Switching
* behavior means switching pipelines.
*  - Pipeline layout — defines the "interface" (descriptor set layouts + push constant ranges) shared between the
* pipeline and shaders.
*  - Dynamic state — select states (viewport, scissor, line width, etc.) can be marked dynamic to avoid creating a new
* pipeline for every variation.
*  - Pipeline cache — allows reuse of compilation results across pipeline creates and even across application runs
* (serializable to disk).
*
* In OpenGL, the driver stitched state together at draw time. Vulkan shifts that cost to creation
* time, making draw calls cheap and predictable — but you pay upfront by creating potentially many pipeline objects.
*/
export module vulkangfx:vulkan.pipeline;
import std;
import :raii;
import :vulkan.exports;
import :vulkan.error;
import :vulkan.raii;

export namespace Vulkan
{
	//
	//
	// Pipeline layout
	struct PipelineLayoutDeleter
	{
		vkr::VkDevice Device = nullptr;
		PipelineLayoutDeleter(vkr::VkDevice device) 
			: Device(device) 
		{
			if (not Device)
				throw VulkanError{ vkr::VkResult::VK_ERROR_INITIALIZATION_FAILED, "Device handle must not be null." };
		}
		constexpr void operator()(this const auto& self, vkr::VkPipelineLayout layout) noexcept
		{
			vkr::vkDestroyPipelineLayout(self.Device, layout, nullptr);
		}
	};
	using PipelineLayoutUniquePtr = std::unique_ptr<std::remove_pointer_t<vkr::VkPipelineLayout>, PipelineLayoutDeleter>;

	struct PipelineLayoutFactory
	{
		vkr::VkDevice Device = nullptr;
		vkr::VkPipelineLayoutCreateInfo CreateInfo{};
		[[nodiscard]]
		auto operator()(this auto& self) -> PipelineLayoutUniquePtr
		{
			if (not self.Device)
				throw VulkanError{ vkr::VkResult::VK_ERROR_INITIALIZATION_FAILED, "Device handle must not be null." };

			self.CreateInfo.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			auto layout = vkr::VkPipelineLayout{};
			auto result = Result{ 
				vkr::vkCreatePipelineLayout(
					self.Device,
					&self.CreateInfo,
					nullptr,
					&layout
				)};
			if (not result)
				throw VulkanError{ result, "Failed to create pipeline layout." };
			return PipelineLayoutUniquePtr{ layout, PipelineLayoutDeleter{ self.Device } };
		}
	};

	class PipelineLayout
	{
	public:
		constexpr PipelineLayout(PipelineLayoutUniquePtr layoutIn)
			: layout(std::move(layoutIn))
		{ }

		constexpr auto GetHandle(this const PipelineLayout& self) noexcept -> vkr::VkPipelineLayout
		{
			return self.layout.get();
		}
	private:
		PipelineLayoutUniquePtr layout;
	};


	//
	//
	// Pipeline
	struct PipelineDeleter
	{
		vkr::VkDevice Device = nullptr;
		PipelineDeleter(vkr::VkDevice device)
			: Device(device)
		{
			if (not Device)
				throw VulkanError{ vkr::VkResult::VK_ERROR_INITIALIZATION_FAILED, "Device handle must not be null." };
		}
		constexpr void operator()(this const auto& self, vkr::VkPipeline pipeline) noexcept
		{
			vkr::vkDestroyPipeline(self.Device, pipeline, nullptr);
		}
	};
	using PipelineUniquePtr = std::unique_ptr<std::remove_pointer_t<vkr::VkPipeline>, PipelineDeleter>;

	struct PipelineFactory
	{
		vkr::VkDevice Device = nullptr;
		vkr::VkGraphicsPipelineCreateInfo CreateInfo{};
		[[nodiscard]]
		auto operator()(this auto& self) -> PipelineUniquePtr
		{
			if (not self.Device)
				throw VulkanError{ vkr::VkResult::VK_ERROR_INITIALIZATION_FAILED, "Device handle must not be null." };
			self.CreateInfo.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			auto pipeline = vkr::VkPipeline{};
			auto result = Result{
				vkr::vkCreateGraphicsPipelines(
					self.Device,
					nullptr,
					1,
					&self.CreateInfo,
					nullptr,
					&pipeline
				)};
			if (not result)
				throw VulkanError{ result, "Failed to create graphics pipeline." };
			return PipelineUniquePtr{ pipeline, PipelineDeleter{ self.Device } };
		}
	};

	class Pipeline
	{
	public:
		Pipeline(PipelineUniquePtr pipelineIn)
			: pipeline(std::move(pipelineIn))
		{}
		constexpr auto GetHandle(this const Pipeline& self) noexcept -> vkr::VkPipeline
		{
			return self.pipeline.get();
		}
	private:
		PipelineUniquePtr pipeline;
	};
}
