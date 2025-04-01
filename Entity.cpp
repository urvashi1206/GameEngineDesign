#include "Entity.h"

#include "Component.h"

Entity::Entity() :
	components()
{
	
}
Entity::~Entity()
{
	for(Component* c : components)
		if(c)
		{
			delete c;
			c = nullptr;
		}

	components.clear();
}

void Entity::AddComponent(Component* component)
{
	components.push_back(component);
	component->SetEntity(this);
}

void Entity::Initialize()
{
	for(Component* c : components)
		c->Initialize();
}
void Entity::Update(float deltaTime)
{
	for(Component* c : components)
		c->Update(deltaTime);
}