export module vulkan26:raii;
import std;
import :error;

export namespace Raii
{
	template <auto VDeleteFn, auto...VArgs>
	struct Deleter
	{
		static constexpr void operator()(auto&& args) noexcept
		{
			VDeleteFn(std::forward<decltype(args)>(args), std::forward<decltype(VArgs)>(VArgs)...);
		}
	};

	template <typename T, auto VDeleteFn, auto...VArgs>
	using DirectUniquePtr = std::unique_ptr<T, Deleter<VDeleteFn, VArgs...>>;
	template <typename T, auto VDeleteFn, auto...VArgs>
	using IndirectUniquePtr = std::unique_ptr<std::remove_pointer_t<T>, Deleter<VDeleteFn, VArgs...>>;

	template<typename T>
	class TypedResource
	{
	public:
		using pointer = typename T::pointer;
		constexpr TypedResource(T resourceIn)
			: resource(std::move(resourceIn))
		{
			if (not resource)
				throw ::Error::RuntimeError{ "Cannot initialise a resource with a null pointer" };
		}
		constexpr auto Get(this const auto& self) noexcept -> pointer
		{
			return self.resource.get();
		}
		constexpr void Destroy(this auto& self) noexcept
		{
			self.resource.reset();
		}
		constexpr auto operator*(this const auto& self) noexcept -> pointer
		{
			return self.resource.get();
		}
		explicit constexpr operator bool(this const auto& self) noexcept
		{
			return static_cast<bool>(self.resource);
		}
	protected:
		T resource;
	};

	static_assert(
		[] consteval -> bool
		{
			auto image = TypedResource<std::unique_ptr<int>>{ std::make_unique<int>(1) };
			if (*image.Get() != 1)
				throw "Unexpected Get() value.";
			if (**image != 1)
				throw "Unexpected operator* value.";
			image.Destroy();
			if (image.Get() != nullptr)
				throw "Expected Get() to return nullptr after resource is destroyed.";
			return true;
		}());
}
