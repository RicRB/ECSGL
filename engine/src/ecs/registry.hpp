#pragma once
#include "scenegraph.hpp"
#include "pool.hpp"
#include "view.hpp"
#include "ecs/system.h"
#include <unordered_map>
#include <queue>
#include <memory>
#include <typeindex>
#include <iostream>
class Registry
{
public:
	SceneGraph scenegraph;
private:
	std::unordered_map<std::type_index, std::shared_ptr<PoolBase>> pools;
	dense_array<Entity, MAX_ENTITIES> entities;
	std::vector<std::shared_ptr<System>> systems; // List of systems
	std::queue<unsigned int> destroyed;
public:

	Registry()
	{
		for (auto& entity : entities.array)
		{
			entity.entityID = ECS_INVALID_ID;
			entity.version = 0;
		}
	}
	// Deconstructor 
	~Registry() = default;

	Registry(Registry const&) = default;
	void operator=(Registry const&) = delete;
private:
	// Constructor

public:
	template <typename ComponentType>
	void AssignComponent(Entity entity, ComponentType component)
	{
		// check if component pool exists
		if (pools.find(typeid(component)) == pools.end())
		{
			// create new pool
			CreatePool<ComponentType>();
		}
		GetPool<ComponentType>()->AddComponent(entity, component);
	}

	template <typename ComponentType>
	void RemoveComponent(Entity entity)
	{
		if (pools.find(typeid(ComponentType)) != pools.end())
		{
			pools[typeid(ComponentType)]->RemoveComponent(entity);
		}
	}

	template <typename ComponentType>
	ComponentType& GetComponent(Entity entity)
	{
		if (pools.find(typeid(ComponentType)) != pools.end())
		{
			// GetPool() and return component
			return GetPool<ComponentType>()->GetComponent(entity);
		}
		else
		{
			throw std::exception("Component does not exist");
		}
	}

	template <typename Type, typename... OtherTypes>
	View<Type, OtherTypes...> GetView()
	{
		// check if pool exists
		if (!PoolExists<Type>())
		{
			std::cout << "Pool 1 does not exist for type: " << typeid(Type).name() << std::endl;
			return View<Type, OtherTypes...>();
		}
		if ((... || !PoolExists<OtherTypes>()))
		{
			std::cout << "Pool 2 does not exist for types: ";
			((std::cout << typeid(OtherTypes).name() << " "), ...);
			std::cout << std::endl;
			return View<Type, OtherTypes...>();
		}
		// Initialize pool Sizes queue
		auto poolSizes = std::queue<std::pair<unsigned int, type_info>>();
		// View 
		View<Type, OtherTypes...> view;

		// Store a pair of smallest size POol and its type
		std::pair<std::type_index, unsigned int> smallestSize = std::make_pair<std::type_index, unsigned int>(typeid(Type), pools[typeid(Type)]->GetSize());
		// iterate through other types and check if they are smaller than the current smallest pool
		(..., (pools[typeid(OtherTypes)]->GetSize() < smallestSize.second ?
			(smallestSize = std::make_pair(std::type_index(typeid(OtherTypes)), GetPool<OtherTypes>()->GetSize())) :
			smallestSize));

		// Get Entities of smallest pool and check which ones have all the components needed
		std::vector<Entity> entities;
		for (auto& pool : pools)
		{
			if (pool.first == smallestSize.first)
			{
				entities = pool.second->GetEntities();
			}
		}
		// Check if entities have all the components and then store them in the view
		using tupleType = std::tuple<std::vector<Entity>, std::vector<Type*>, std::vector<OtherTypes*>...>;
		tupleType componentTuples;
		for (auto& entity : entities)
		{
			// Check if Component exists in entity
			if ((... && ComponentExists<OtherTypes>(entity)))
			{
				// Add Entity and its components to tuple
				std::get<0>(componentTuples).push_back(entity);
				std::get<1>(componentTuples).push_back(&GetComponent<Type>(entity));
				(..., std::get<std::vector<OtherTypes*>>(componentTuples).push_back(&GetComponent<OtherTypes>(entity)));
			}
		}
		view.SetData(componentTuples);
		return view;
	}


	template <typename ComponentType>
	bool PoolExists()
	{
		bool exists = (pools.find(typeid(ComponentType)) != pools.end());
		return exists;
	}
	template <typename ComponentType>
	bool ComponentExists(Entity entity)
	{
		if (pools.find(typeid(ComponentType)) != pools.end())
		{
			return GetPool<ComponentType>()->ComponentExists(entity);
		}
		else
		{
			throw std::exception("Component does not exist");
		}
	}
	const std::vector<Entity> GetActiveEntities()
	{
		std::vector<Entity> activeEntities;
		for (unsigned int i = 0; i < entities.size(); i++)
		{
			if (entities[i].entityID != ECS_INVALID_ID)
			{
				activeEntities.push_back(entities[i]);
			}
		}
		return activeEntities;
	}
	Entity CreateEntity()
	{
		Entity entity;
		if (destroyed.empty())
		{
			entities.nextEmpty().entityID = entities.size();
			entity = entities.nextEmpty();
			entities.size()++;
		}
		else
		{
			entities[destroyed.front()].entityID = destroyed.front();
			entity = entities[destroyed.front()];
			destroyed.pop();
		}
		return entity;
	}

	void DestroyEntity(Entity entity)
	{
		destroyed.push(entity.entityID);
		entities[entity.entityID].entityID = ECS_INVALID_ID;
		entities[entity.entityID].version++;
		// TODO: Get Children and delete aswell
		for (auto& pool : pools)
		{
			pool.second->RemoveComponent(entity);
		}
	}
	void Clear()
	{
		pools.clear();
		entities = dense_array<Entity, MAX_ENTITIES>();
		destroyed = std::queue<unsigned int>();
	}

	glm::mat4 GetLocalToWorldMatrix(Entity entity)
	{
		return scenegraph.GetLocalToWorldMatrix(*this, entity);
	}
	template <typename SystemType>
	void AddSystem()
	{
		static_assert(std::is_base_of<System, SystemType>::value, "SystemType must inherit from System");
		// Check if system already exists
		for (const auto& system : systems)
		{
			if (typeid(*system) == typeid(SystemType))
			{
				std::cout << "System of type " << typeid(SystemType).name() << " already exists." << std::endl;
				return;
			}
		}

		systems.push_back(std::make_shared<SystemType>());
	}
	void startSystems()
	{
		for (const auto& system : systems)
		{
			system->start();
		}
	}
	void updateSystems()
	{
		for (const auto& system : systems)
		{
			system->update();
		}
	}
private:
	template <typename ComponentType>
	Pool<ComponentType>* GetPool()
	{
		auto it = pools.find(typeid(ComponentType));
		if (it != pools.end())
			return dynamic_cast<Pool<ComponentType>*>(it->second.get());
		else
			throw std::runtime_error("Pool does not exist" + std::string(typeid(ComponentType).name()));
	}
	// a function that creates a pool and adds it to the map
	template <typename ComponentType>
	void CreatePool()
	{
		// check if pool exists
		if (pools.find(typeid(ComponentType)) == pools.end())
		{
			// cast pool into base pool and store it in map
			std::unique_ptr<PoolBase> base = std::make_unique<Pool<ComponentType>>();
			pools[typeid(ComponentType)] = std::move(base);
		}
	}
};
