#pragma once
#include <array>
template <typename T, size_t n>
struct MemoryPool {
	MemoryPool();
	~MemoryPool();
	template<typename... Args>
	const T* allocate(Args&&... args) {
		uint32_t index = firstEmpty;
		pool[firstEmpty] = T(std::forward<Args>(args)...);
		if (firstEmpty < lastAllocated)
			firstEmpty = lastAllocated++;
		else
			firstEmpty++;

		lastAllocated = firstEmpty;

		return index;
	}
	void deallocate(uint32_t index) {
		if (index >= n) return;
		if (index < pool.size()) {
			~pool[index];
			firstEmpty = index;
		}
	}

private:
	std::array<T, n> pool;
	uint32_t firstEmpty = 0;
	uint32_t lastAllocated = 0;
};