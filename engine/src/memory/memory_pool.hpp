#pragma once
#include <array>
#include <stack>

template <typename T>
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
	MemoryHandle<T> allocate(Args&&... args) {
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
				return MemoryHandle<T>(-1);
			}
		}

		pool[index] = T(std::forward<Args>(args)...);

		return index;
	}

	void deallocate(MemoryHandle<T>& memoryHandle) {
		uint32_t& index = memoryHandle.m_index;
		if (index >= n) {
			throw std::runtime_error("MemoryPool: Invalid handle");
			return;
		}
		destroyedIndices.push(index);
		pool[index].~T();
	}

	T& operator[](const MemoryHandle<T>& handle) {
		return pool[handle.m_index];
	}

	const T& operator[](const MemoryHandle<T>& handle) const {
		return pool[handle.m_index];
	}

	auto begin() { return pool.begin(); }
	auto end() { return pool.end(); }
	auto begin() const { return pool.begin(); }
	auto end()   const { return pool.end(); }
	const size_t size() const {
		return allocationPointer - destroyedIndices.size();
	}

	void clear() {
		for (size_t i = 0; i < allocationPointer; i++)
		{
			pool[i].~T();
		}
		allocationPointer = 0;
		while (!destroyedIndices.empty()) destroyedIndices.pop();
	}
private:
	std::array<T, n> pool;
	uint32_t allocationPointer = 0;
	std::stack<uint32_t> destroyedIndices;
};