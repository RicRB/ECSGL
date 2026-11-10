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
	std::unordered_map<std::type_index, std::unique_ptr<PoolBase>> pools;
	std::unique_ptr<dense_array<Entity, MAX_ENTITIES>> entities;
	std::vector<std::shared_ptr<System>> systems; // List of systems
	std::queue<unsigned int> destroyed;
public:

	Registry()
	{
		entities = std::make_unique<dense_array<Entity, MAX_ENTITIES>>();
		for (auto& entity : entities->array)
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
		return View<Type, OtherTypes...>(GetPool<Type>(), GetPool<OtherTypes>()...);
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
	Entity GetEntity(uint32_t entityID) const
	{
		if (entityID >= entities->size()) return Entity();
		return (*entities)[entityID];
	}
	Entity CreateEntity()
	{
		Entity entity;
		if (destroyed.empty())
		{
			entities->nextEmpty().entityID = entities->size();
			entity = entities->nextEmpty();
			entities->size()++;
		}
		else
		{
			(*entities)[destroyed.front()].entityID = destroyed.front();
			entity = (*entities)[destroyed.front()];
			destroyed.pop();
		}
		return entity;
	}

	void DestroyEntity(Entity entity)
	{
		destroyed.push(entity.entityID);
		(*entities)[entity.entityID].entityID = ECS_INVALID_ID;
		(*entities)[entity.entityID].version++;
		// TODO: Get Children and delete aswell
		for (auto& pool : pools)
		{
			pool.second->RemoveComponent(entity);
		}
	}
	void Clear()
	{
		pools.clear();
		entities.reset();
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

	void guiSystems()
	{
		for (const auto& system : systems)
		{
			system->onGui();
		}
	}

	void cleanupSystems()
	{
		for (const auto& system : systems)
		{
			system->cleanup();
		}
		systems.clear();
	}
private:
	template <typename ComponentType>
	Pool<ComponentType>* GetPool()
	{
		auto it = pools.find(typeid(ComponentType));
		if (it != pools.end())
			return static_cast<Pool<ComponentType>*>(it->second.get());
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
			pools[typeid(ComponentType)] = std::make_unique<Pool<ComponentType>>();
		}
	}

	bool IsAlive(Entity entity) const {
		if (entity.entityID >= entities->size())
			return false;

		const Entity& current = (*entities)[entity.entityID];

		return current.entityID != ECS_INVALID_ID && current.version == entity.version;
	}
};
