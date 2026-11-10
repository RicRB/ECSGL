#pragma once
#include "entity.h"
#include "pool.hpp"
#include <tuple>
#include <vector>
#include <utility>

template <typename... ComponentTypes>
class View
{
private:
	std::tuple<Pool<ComponentTypes>*...> pools{};
	const Entity* leadEntities = nullptr;
	unsigned int leadSize = 0;

	mutable std::tuple<std::vector<Entity>, std::vector<ComponentTypes*>...> components;
	mutable bool cached = false;
	static constexpr size_t value = sizeof...(ComponentTypes);

	bool matches(const Entity entity) const
	{
		return entity.entityID < MAX_ENTITIES && (std::get<Pool<ComponentTypes>*>(pools)->ComponentExists(entity) && ...);
	}

	void buildCache() const
	{
		if (cached) return;
		cached = true;
		std::get<0>(components).reserve(leadSize);
		(std::get<std::vector<ComponentTypes*>>(components).reserve(leadSize), ...);
		for (unsigned int i = 0; i < leadSize; i++)
		{
			const Entity entity = leadEntities[i];
			if (!matches(entity)) continue;
			std::get<0>(components).push_back(entity);
			(std::get<std::vector<ComponentTypes*>>(components).push_back(&std::get<Pool<ComponentTypes>*>(pools)->GetComponent(entity)), ...);
		}
	}

public:
	View() = default;

	explicit View(Pool<ComponentTypes>*... p) : pools(p...)
	{
		auto pick = [this](auto* pool) {
			if (!leadEntities || pool->GetSize() < leadSize) {
				leadEntities = pool->entities();
				leadSize = pool->GetSize();
			}
		};
		(pick(p), ...);
	}

	template <typename Func>
	void each(Func&& func)
	{
		for (unsigned int i = leadSize; i-- > 0;)
		{
			const Entity entity = leadEntities[i];
			if (matches(entity))
			{
				func(entity, std::get<Pool<ComponentTypes>*>(pools)->GetComponent(entity)...);
			}
		}
	}

	unsigned int sizeHint() const
	{
		return leadSize;
	}

	void SetData(const std::tuple<std::vector<Entity>, std::vector<ComponentTypes*>...> data)
	{
		components = data;
		cached = true;
	}

	std::vector<Entity>& GetEntities()
	{
		buildCache();
		return std::get<0>(components);
	}

	template <typename RequestedType>
	std::vector<RequestedType*>& GetComponents()
	{
		buildCache();
		return std::get<std::vector<RequestedType*>>(components);
	}

	template <typename RequestedType>
	RequestedType* GetComponent(const Entity entity)
	{
		Pool<RequestedType>* pool = std::get<Pool<RequestedType>*>(pools);
		if (!pool || !matches(entity)) return nullptr;
		return &pool->GetComponent(entity);
	}

	template <typename RequestedType>
	RequestedType* GetComponent(const size_t index)
	{
		buildCache();
		return std::get<std::vector<RequestedType*>>(components)[index];
	}

	Entity GetEntity(const size_t index)
	{
		buildCache();
		return std::get<0>(components)[index];
	}

	template <typename RequestedType>
	std::pair<Entity, RequestedType*> operator[](unsigned int index)
	{
		buildCache();
		return { std::get<0>(components)[index], std::get<std::vector<RequestedType*>>(components)[index] };
	}

	template <typename... RequestedTypes>
	std::tuple<Entity, RequestedTypes*...> Get(unsigned int index)
	{
		buildCache();
		return { std::get<0>(components)[index], std::get<std::vector<RequestedTypes*>>(components)[index]... };
	}

	std::tuple<Entity, ComponentTypes*...> operator[](unsigned int index)
	{
		buildCache();
		return { std::get<0>(components)[index], std::get<std::vector<ComponentTypes*>>(components)[index]... };
	}

	const size_t size() const
	{
		if constexpr (sizeof...(ComponentTypes) == 1)
		{
			if (!cached) return leadSize;
		}
		buildCache();
		return std::get<0>(components).size();
	}
};
