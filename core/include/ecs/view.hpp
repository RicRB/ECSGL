#pragma once
#include "entity.h"
#include <vector>

template <typename... ComponentTypes>
class View
{
private:
	std::tuple<std::vector<Entity>, std::vector<ComponentTypes*>...> components;
	static constexpr size_t value = sizeof...(ComponentTypes);
public:
	View() {
	};
	~View() {};
	void SetData(const std::tuple<std::vector<Entity>, std::vector<ComponentTypes*>...> data)
	{
		components = data;
	}
	std::vector<Entity>& GetEntities()
	{
		return std::get<0>(components);
	}
	template <typename RequestedType>
	std::vector<RequestedType*>& GetComponents()
	{
		try
		{
			return std::get<std::vector<RequestedType*>>(components);
		}
		catch (const std::exception&)
		{
			throw std::exception("Component type not found");
		}
	}
	template <typename RequestedType>
	RequestedType* GetComponent(const Entity entity)
	{
		std::vector<Entity>& entities = std::get<0>(components);
		std::vector<RequestedType*>& componentVector = std::get<std::vector<RequestedType*>>(components);
		for (size_t i = 0; i < entities.size(); i++)
		{
			if (entities[i] == entity)
			{
				return componentVector[i];
			}
		}
		return nullptr;
	}
	template <typename RequestedType>
	std::pair<Entity, RequestedType*> operator[](unsigned int index)
	{
		std::pair<Entity, RequestedType*> pair;
		pair.first = std::get<0>(components)[index];
		pair.second = std::get<std::vector<RequestedType*>>(components)[index];
		return pair;
	}
	template <typename... RequestedTypes>
	std::tuple<Entity, RequestedTypes*...> get(unsigned int index) {
		std::tuple<Entity, RequestedTypes*...> result;

		// Assuming components is a tuple containing data of various types
		// You would need to adjust this part to retrieve the data accordingly
		std::get<0>(result) = std::get<0>(components)[index];
		int _[] = { 0, ((std::get<RequestedTypes*>(result) = std::get<std::vector<RequestedTypes*>>(components)[index]), 0)... };
		return result;
	}

	std::tuple<Entity, ComponentTypes*...> operator[](unsigned int index) {
		std::tuple<Entity, ComponentTypes*...> result;

		// Assuming components is a tuple containing data of various types
		// You would need to adjust this part to retrieve the data accordingly
		std::get<0>(result) = std::get<0>(components)[index];
		int _[] = { 0, ((std::get<ComponentTypes*>(result) = std::get<std::vector<ComponentTypes*>>(components)[index]), 0)... };
		return result;
	}
	size_t size() const
	{
		return std::get<0>(components).size();
	}
};