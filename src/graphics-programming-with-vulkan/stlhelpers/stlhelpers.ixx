export module vulkangfx:stlhelpers;
import std;

export namespace StlHelpers
{
	template<typename T>
	auto ToVector(std::span<T> span) -> std::vector<T>
	{
		return std::vector<T>{ span.begin(), span.end() };
	}

	template<std::ranges::range T>
	struct Collection;

	template<typename F>
	struct FilterClosure { F predicate; };

	template<typename F>
	struct TransformClosure { F transformer; };

	struct CollectClosure {};

	constexpr auto filter(auto&& predicate)
	{
		return FilterClosure{ std::forward<decltype(predicate)>(predicate) };
	}

	constexpr auto transform(auto&& transformer)
	{
		return TransformClosure{ std::forward<decltype(transformer)>(transformer) };
	}

	constexpr auto collect() -> CollectClosure
	{
		return {};
	}

	template<std::ranges::range T>
	struct Collection
	{
		constexpr Collection(T&& items)
			: collection(std::move(items))
		{ }

		constexpr Collection(const T& items)
			: collection(items)
		{ }

		constexpr auto begin(this auto&& self) noexcept -> decltype(std::begin(self.collection))
		{
			return std::begin(self.collection);
		}

		constexpr auto end(this auto&& self) noexcept -> decltype(std::end(self.collection))
		{
			return std::end(self.collection);
		}

		constexpr auto empty() const noexcept -> bool
		{
			return std::empty(collection);
		}

		constexpr auto any_of(this const Collection& self, auto&& predicate) -> bool
		{
			return std::ranges::any_of(self.collection, std::forward<decltype(predicate)>(predicate));
		}

		constexpr auto none_of(this const Collection& self, auto&& predicate) -> bool
		{
			return std::ranges::none_of(self.collection, std::forward<decltype(predicate)>(predicate));
		}

		constexpr auto all_of(this const Collection& self, auto&& predicate) -> bool
		{
			return std::ranges::all_of(self.collection, std::forward<decltype(predicate)>(predicate));
		}

		constexpr auto count_if(this const Collection& self, auto&& predicate) -> std::ranges::range_difference_t<T>
		{
			return std::ranges::count_if(self.collection, std::forward<decltype(predicate)>(predicate));
		}

		constexpr auto filter(this const Collection& self, auto&& predicate) -> Collection<std::vector<std::ranges::range_value_t<T>>>
		{
			auto filtered = std::vector<std::ranges::range_value_t<T>>{};
			std::ranges::copy_if(
				self.collection,
				std::back_inserter(filtered),
				std::forward<decltype(predicate)>(predicate)
			);
			return Collection{ std::move(filtered) };
		}

		constexpr auto erase_if(this Collection& self, auto&& predicate) -> void
		{
			self.collection.erase(
				std::remove_if(
					self.collection.begin(),
					self.collection.end(),
					std::forward<decltype(predicate)>(predicate)
				),
				self.collection.end()
			);
		}

		constexpr auto transform(this const Collection& self, auto&& transformer) -> Collection<std::vector<std::invoke_result_t<decltype(transformer), std::ranges::range_value_t<T>>>>
		{
			using TransformedType = std::invoke_result_t<decltype(transformer), std::ranges::range_value_t<T>>;
			auto transformed = std::vector<TransformedType>{};
			std::ranges::transform(
				self.collection,
				std::back_inserter(transformed),
				std::forward<decltype(transformer)>(transformer)
			);
			return Collection{ std::move(transformed) };
		}

		constexpr operator T& () noexcept
		{
			return collection;
		}

		constexpr operator const T& () const noexcept
		{
			return collection;
		}

		T collection;
	};

	template<std::ranges::range T, typename F>
	constexpr auto operator|(const Collection<T>& col, FilterClosure<F> closure)
	{
		return col.filter(std::move(closure.predicate));
	}

	template<std::ranges::range T, typename F>
	constexpr auto operator|(const Collection<T>& col, TransformClosure<F> closure)
	{
		return col.transform(std::move(closure.transformer));
	}

	template<std::ranges::range R>
	constexpr auto operator|(R&& range, CollectClosure)
	{
		using Value = std::ranges::range_value_t<std::remove_cvref_t<R>>;
		auto vec = std::vector<Value>{ std::ranges::begin(range), std::ranges::end(range) };
		return Collection<std::vector<Value>>{ std::move(vec) };
	}

	static_assert(
		[] {
			auto numbers = Collection{ std::vector{ 1, 2, 3, 4, 5 } };
			auto evenNumbers = numbers | std::ranges::views::filter([](int n) { return n % 2 == 0; });
			auto squaredNumbers = numbers | std::ranges::views::transform([](int n) { return n * n; });

			if(not std::ranges::equal(evenNumbers | std::ranges::to<std::vector>(), std::vector{2, 4}))
				return false;
			if (not std::ranges::equal(squaredNumbers | std::ranges::to<std::vector>(), std::vector{ 1, 4, 9, 16, 25 }))
				return false;

			/*if(not std::ranges::equal(evenNumbers.collection, std::vector{ 2, 4 }))
				return false;
			if (not std::ranges::equal(squaredNumbers.collection, std::vector{ 1, 4, 9, 16, 25 }))
				return false;*/
			return true;
		}()
	);
}
