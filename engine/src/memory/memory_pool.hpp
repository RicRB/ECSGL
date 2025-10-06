#pragma once
#include <array>
#include <stack>
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
		uint32_t index;
		if (!destroyedIndices.empty()) {
			index = destroyedIndices.top();
			destroyedIndices.pop();
		}
		else
		{
			index = allocationPointer++;
			if (index >= n) {
				throw std::runtime_error("MemoryPool: Out of memory");
				return MemoryHandle(-1);
			}
		}

		pool[index] = T(std::forward<Args>(args)...);

		return index;
	}
	void deallocate(MemoryHandle& memoryHandle) {
		uint32_t& index = memoryHandle.m_index;
		if (index >= n) {
			throw std::runtime_error("MemoryPool: Invalid handle");
			return;
		}
		destroyedIndices.push(index);
		pool[index].~T();
	}

	T& operator[](const MemoryHandle& handle) {
		return pool[handle.m_index];
	}

	const T& operator[](const MemoryHandle& handle) const {
		return pool[handle.m_index];
	}
private:
	std::array<T, n> pool;
	uint32_t allocationPointer = 0;
	std::stack<uint32_t> destroyedIndices;
};