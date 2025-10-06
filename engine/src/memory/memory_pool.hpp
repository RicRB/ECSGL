#pragma once
#include <array>
class MemoryHandle {
public:
	MemoryHandle(const uint32_t idx) : m_index(idx) {};
	const uint32_t m_index;
public:
	bool operator==(const MemoryHandle& other) const {
		return m_index == other.m_index;
	}
};

template <typename T, size_t n>
struct MemoryPool {
	MemoryPool() = default;
	~MemoryPool() = default;
	template<typename... Args>
	MemoryHandle allocate(Args&&... args) {
		uint32_t index = firstEmpty;
		pool[firstEmpty] = T(std::forward<Args>(args)...);
		if (firstEmpty < lastAllocated)
			firstEmpty = lastAllocated++;
		else
			firstEmpty++;

		lastAllocated = firstEmpty;

		return index;
	}
	void deallocate(MemoryHandle& memoryHandle) {
		uint32_t& index = memoryHandle.m_index;
		if (index >= n) return;
		if (index < pool.size()) {
			~pool[index];
			firstEmpty = index;
		}
	}

	T& operator[](const MemoryHandle& handle) {
		return pool[handle.m_index];
	}

	const T& operator[](const MemoryHandle& handle) const {
		return pool[handle.m_index];
	}

	const bool isValid(const MemoryHandle& handle) const {
		// TODO: FIGURE OUT THE LOGIC FOR THIS.
		bool valid = handle.m_index < n;
		
		return ;
	}
private:
	std::array<T, n> pool;
	uint32_t firstEmpty = 0;
	uint32_t lastAllocated = 0;
};