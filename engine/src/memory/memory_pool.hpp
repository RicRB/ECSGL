#pragma once
#include <array>
#include <stack>
#include <stdint.h>
#include <limits>

class MemoryHandle {
public:
	static constexpr uint32_t HANDLE_INVALID_INDEX = std::numeric_limits<uint32_t>::max();

	uint32_t index;
public:
	MemoryHandle(const uint32_t idx = HANDLE_INVALID_INDEX) : index(idx) {};

	const bool isValid() const {
		return index != HANDLE_INVALID_INDEX;
	}
	bool operator==(const MemoryHandle& other) const {
		return index == other.index;
	}
	operator uint32_t() const {
		return index;
	}

};

namespace std
{
	template<>
	struct hash<MemoryHandle> {
		size_t operator()(const MemoryHandle& h) const noexcept {
			return std::hash<uint32_t>{}(h.index);
		}
	};
}
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
		uint32_t& index = memoryHandle.index;
		if (index >= n) {
			throw std::runtime_error("MemoryPool: Invalid handle");
			return;
		}
		destroyedIndices.push(index);
		pool[index].~T();
	}

	T& operator[](const MemoryHandle& handle) {
		return pool[handle.index];
	}

	const T& operator[](const MemoryHandle& handle) const {
		return pool[handle.index];
	}

	T& at(const MemoryHandle& handle) {
		return pool.at(handle.index);
	}
	
	T& at(uint32_t index) {
		return pool.at(index);
	}

	const T& at(const MemoryHandle& handle) const {
		return pool.at(handle.index);
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