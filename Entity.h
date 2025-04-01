#pragma once

#include <vector>

class Component;

class Entity
{
protected:
	std::vector<Component*> components;

public:
	Entity();
	~Entity();

	void AddComponent(Component* component);
	
	template<class T>
	T* GetComponent();

protected:
	virtual void Initialize();
	virtual void Update(float deltaTime);

	friend class EntityManager;

public:
	std::vector<Component*> GetComponents() const { return components; };
};

template<class T>
inline T* Entity::GetComponent()
{
	for(Component* c : components)
		if(T* t = dynamic_cast<T*>(c))
			return t;

	return nullptr;
}