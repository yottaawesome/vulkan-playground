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

	template<std::ranges::range T>
	Collection(T&&) -> Collection<std::remove_cvref_t<T>>;
	template<std::ranges::range T>
	Collection(Collection<T>&&) -> Collection<std::remove_cvref_t<T>>;

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

	struct NullMutex
	{
		void lock(this const NullMutex&) noexcept { }
		void unlock(this const NullMutex&) noexcept { }
		auto try_lock(this const NullMutex&) noexcept -> bool { return true; }
	};

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

		constexpr auto operator[](this auto&& self, size_t x) -> decltype(auto)
		{
			return std::forward_like<decltype(self)>(self.collection)[x];
		}

		constexpr auto end(this auto&& self) noexcept -> decltype(std::end(self.collection))
		{
			return std::end(self.collection);
		}

		constexpr auto empty(this const Collection& self) noexcept -> bool
		{
			return std::empty(self.collection);
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

		constexpr auto front(this auto&& self) -> decltype(auto)
		{
			return std::forward_like<decltype(self)>(self.collection).front();
		}

		constexpr auto back(this auto&& self) -> decltype(auto)
		{
			return std::forward_like<decltype(self)>(self.collection).back();
		}

		constexpr auto find_if(this const Collection& self, auto&& predicate) -> std::optional<std::ranges::range_value_t<T>>
		{
			auto it = std::ranges::find_if(self.collection, std::forward<decltype(predicate)>(predicate));
			if (it == std::ranges::end(self.collection))
				return std::nullopt;
			return *it;
		}

		constexpr auto contains(this const Collection& self, const std::ranges::range_value_t<T>& value) -> bool
		{
			return std::ranges::find(self.collection, value) != std::ranges::end(self.collection);
		}

		constexpr auto contains_if(this const Collection& self, auto&& predicate) -> bool
		{
			return std::ranges::any_of(self.collection, std::forward<decltype(predicate)>(predicate));
		}

		template<typename U, typename BinaryOp>
		constexpr auto fold_left(this const Collection& self, U init, BinaryOp op) -> U
		{
			return std::ranges::fold_left(self.collection, std::move(init), std::move(op));
		}

		constexpr auto flat_map(this const Collection& self, auto&& transformer) -> Collection<std::vector<std::ranges::range_value_t<std::invoke_result_t<decltype(transformer), std::ranges::range_value_t<T>>>>>
		{
			using InnerRange = std::invoke_result_t<decltype(transformer), std::ranges::range_value_t<T>>;
			using Value = std::ranges::range_value_t<InnerRange>;
			auto result = std::vector<Value>{};
			for (const auto& item : self.collection)
			{
				auto inner = std::invoke(std::forward<decltype(transformer)>(transformer), item);
				result.insert(result.end(), std::ranges::begin(inner), std::ranges::end(inner));
			}
			return Collection{ std::move(result) };
		}

		constexpr auto sorted(this const Collection& self) -> Collection<std::vector<std::ranges::range_value_t<T>>>
		{
			auto copy = std::vector<std::ranges::range_value_t<T>>{ std::ranges::begin(self.collection), std::ranges::end(self.collection) };
			std::ranges::sort(copy);
			return Collection{ std::move(copy) };
		}

		constexpr auto sorted_by(this const Collection& self, auto&& comparator) -> Collection<std::vector<std::ranges::range_value_t<T>>>
		{
			auto copy = std::vector<std::ranges::range_value_t<T>>{ std::ranges::begin(self.collection), std::ranges::end(self.collection) };
			std::ranges::sort(copy, std::forward<decltype(comparator)>(comparator));
			return Collection{ std::move(copy) };
		}

		constexpr auto take(this const Collection& self, std::size_t n) -> Collection<std::vector<std::ranges::range_value_t<T>>>
		{
			auto count = std::min(n, static_cast<std::size_t>(std::ranges::size(self.collection)));
			auto result = std::vector<std::ranges::range_value_t<T>>{
				std::ranges::begin(self.collection),
				std::ranges::begin(self.collection) + static_cast<std::ranges::range_difference_t<T>>(count)
			};
			return Collection{ std::move(result) };
		}

		constexpr auto skip(this const Collection& self, std::size_t n) -> Collection<std::vector<std::ranges::range_value_t<T>>>
		{
			auto count = std::min(n, static_cast<std::size_t>(std::ranges::size(self.collection)));
			auto result = std::vector<std::ranges::range_value_t<T>>{
				std::ranges::begin(self.collection) + static_cast<std::ranges::range_difference_t<T>>(count),
				std::ranges::end(self.collection)
			};
			return Collection{ std::move(result) };
		}

		constexpr auto size(this const auto& self) noexcept -> decltype(std::ranges::size(self.collection))
		{
			return std::ranges::size(self.collection);
		}

		constexpr auto underlying(this auto&& self) noexcept -> decltype(auto)
		{
			return std::forward_like<decltype(self)>(self.collection);
		}

	private:
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

	template<typename T>
	using Vector = Collection<std::vector<T>>;

	static_assert(
		[] {
			// Standard range pipe compatibility.
			auto numbers = Collection{ std::vector{ 1, 2, 3, 4, 5 } };
			auto evenNumbers = numbers | std::ranges::views::filter([](int n) { return n % 2 == 0; });
			auto squaredNumbers = numbers | std::ranges::views::transform([](int n) { return n * n; });

			if (not std::ranges::equal(evenNumbers | std::ranges::to<std::vector>(), std::vector{2, 4}))
				return false;
			if (not std::ranges::equal(squaredNumbers | std::ranges::to<std::vector>(), std::vector{ 1, 4, 9, 16, 25 }))
				return false;

			// CTAD from lvalue (should copy, not create a reference member).
			auto vec = std::vector{ 10, 20, 30 };
			auto fromLvalue = Collection{ vec };
			if (fromLvalue.size() != 3)
				return false;

			// Collection-preserving pipes.
			auto evens = numbers
				| StlHelpers::filter([](int n) { return n % 2 == 0; })
				| StlHelpers::transform([](int n) { return n * 10; });
			if (not std::ranges::equal(evens.underlying(), std::vector{ 20, 40 }))
				return false;

			// collect() materialises a range into a Collection.
			auto source = std::vector{ 4, 5 };
			auto collected = source | StlHelpers::collect();
			if (not std::ranges::equal(collected.underlying(), std::vector{ 4, 5 }))
				return false;

			// front() / back().
			if (numbers.front() != 1)
				return false;
			if (numbers.back() != 5)
				return false;

			// find_if — found.
			auto found = numbers.find_if([](int n) { return n == 3; });
			if (not found.has_value() or found.value() != 3)
				return false;

			// find_if — not found.
			auto notFound = numbers.find_if([](int n) { return n == 99; });
			if (notFound.has_value())
				return false;

			// contains / contains_if.
			if (not numbers.contains(3))
				return false;
			if (numbers.contains(99))
				return false;
			if (not numbers.contains_if([](int n) { return n > 4; }))
				return false;
			if (numbers.contains_if([](int n) { return n > 10; }))
				return false;

			// fold_left.
			auto sum = numbers.fold_left(0, std::plus<>{});
			if (sum != 15)
				return false;

			// flat_map.
			auto pairs = Collection{ std::vector{ 1, 2, 3 } };
			auto flatMapped = pairs.flat_map([](int n) { return std::vector{ n, n * 10 }; });
			if (not std::ranges::equal(flatMapped.underlying(), std::vector{ 1, 10, 2, 20, 3, 30 }))
				return false;

			// sorted.
			auto unsorted = Collection{ std::vector{ 3, 1, 4, 1, 5 } };
			auto asc = unsorted.sorted();
			if (not std::ranges::equal(asc.underlying(), std::vector{ 1, 1, 3, 4, 5 }))
				return false;

			// sorted_by (descending).
			auto desc = unsorted.sorted_by(std::greater<>{});
			if (not std::ranges::equal(desc.underlying(), std::vector{ 5, 4, 3, 1, 1 }))
				return false;

			// take / skip.
			if (not std::ranges::equal(numbers.take(3).underlying(), std::vector{ 1, 2, 3 }))
				return false;
			if (not std::ranges::equal(numbers.skip(3).underlying(), std::vector{ 4, 5 }))
				return false;

			// take/skip beyond bounds.
			if (numbers.take(100).size() != 5)
				return false;
			if (not numbers.skip(100).empty())
				return false;

			return true;
		}()
	);
}
