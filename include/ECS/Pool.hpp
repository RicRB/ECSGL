#pragma once
#include "entity.h"
#include <array>
#include <stdexcept>
#include <vector>
#include <tuple>
#define INVALID_INDEX 0xFFFFFFFF


template <typename T, std::size_t n>
struct dense_array
{
public:
	std::array<T, n> array;
private:
	unsigned int mSize = 0;
public:
	dense_array() = default;
	T& operator[](unsigned int index) {
		return array[index];
	}
	void operator=(const dense_array& other) {
		array = other.array;
		mSize = other.mSize;
	}
	const std::vector<T> vector() const {
		std::vector<T> vec;
		for (unsigned int i = 0; i < mSize; i++)
		{
			vec.push_back(array[i]);
		}
		return vec;
	}
	void push_back(T element) {
		if (full())
			throw std::overflow_error("Array is full");

		array[mSize] = element;
		mSize++;
		return;
	}
	void removeAt(const unsigned int index) {
		// if empty
		if (empty()){
			throw std::out_of_range("array Empty");
			return;
		}
		// if index is last element
		if (index == size() - 1)
		{
			// clear the last element
			array[mSize-1] = T();
			mSize--;
			return;
		}
		// if index is not last element
		if (index < mSize-1) {
			array[index] = array[mSize-1];
			// clear the last element
			array[mSize-1] = T();
			mSize--;
			return;
		}
	}
	T* begin() {
		return &array[0];
	}

	T* end() {
		return &array[mSize];
	}


	T& last()
	{
		return array[mSize-1];
	}
	T& nextEmpty()
	{
		if (full()) 	
			throw std::overflow_error("Array is full");
		return array[mSize];
	}
	const bool full() const {
		return mSize == n;
	}
	const bool empty() const {
		return mSize == 0;
	}
	const unsigned int size() const {
		return mSize;
	}
	unsigned int& size() {
		return mSize;
	}
};

// Base class for component pools
class PoolBase {
public:
	PoolBase() {};
	virtual ~PoolBase() = default;
	virtual void clear(){}
	virtual void RemoveComponent(Entity entity) {}
	virtual bool ComponentExists(Entity entity) { return false; }
	virtual const std::vector<Entity> GetEntities() const { return std::vector<Entity>(); }
	virtual const unsigned int GetSize() const { return MAX_ENTITIES+1; }
};

template <typename ComponentType>
class Pool : public PoolBase
{
private:
	dense_array<ComponentType, MAX_ENTITIES> componentList;
	dense_array<bool, MAX_ENTITIES> activeList;
	dense_array<Entity, MAX_ENTITIES> entityList;
	std::array<unsigned int, MAX_ENTITIES> entityIndices;
public:
	Pool()
	{
		for (int i = 0; i < MAX_ENTITIES; i++)
		{
			entityIndices[i] = INVALID_INDEX;
			activeList[i] = false;
		}
	}
	~Pool() = default;
	// i dont know if this actually releases memory but oh well we wouldnt ever need to clear this
	void clear() override
	{
		for (int i = 0; i < MAX_ENTITIES; i++)
		{
			entityIndices[i] = INVALID_INDEX;
			activeList[i] = false;
		}
		componentList.size() = 0;
		entityList.size() = 0;
	}
	void AddComponent(Entity entity, ComponentType component)
	{
		// add entity to the back of entitylist
		unsigned int index = entityIndices[entity.entityID];
		if (index == INVALID_INDEX)
		{
			entityList.push_back(entity);
			componentList.push_back(component);
			activeList.push_back(true);
			entityIndices[entity.entityID] = entityList.size()-1;
		}
	}
	void RemoveComponent(Entity entity) override
	{
		unsigned int index = entityIndices[entity.entityID];
		if (index != INVALID_INDEX)
		{
			if (index == componentList.size() - 1)
			{
				// if last index just remove
				entityIndices[entity.entityID] = INVALID_INDEX;
				componentList.removeAt(index);
				entityList.removeAt(index);
				activeList.removeAt(index);
			}
			else
			{
				// repacking arrays
				unsigned int movedEntityID = entityList.last().entityID;
				entityList.removeAt(index);
				componentList.removeAt(index);
				activeList.removeAt(index);
				entityIndices[movedEntityID] = index;
			}
		}
	}
	ComponentType& GetComponent(Entity entity)
	{
		if (ComponentExists(entity))
		{
			return componentList[entityIndices[entity.entityID]];
		}
		throw std::out_of_range("Component does not exist");
	}
	std::vector<std::pair<Entity&, ComponentType&>> GetComponents()
	{
		std::vector<std::pair<Entity&, ComponentType&>> components;
		for (int i = 0; i < componentList.mSize+1; i++)
		{
			components.push_back(std::make_pair(entityList[i], componentList[i]));
		}
		return components;
	}
	const bool ComponentExists(Entity entity) const
	{
		return entityIndices[entity.entityID] != INVALID_INDEX;
	}
	const std::vector<Entity> GetEntities() const
	{
		return entityList.vector();
	}
	const unsigned int GetSize() const
	{
		return componentList.size();
	}

};