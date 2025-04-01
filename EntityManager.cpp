#include "EntityManager.h"

namespace
{
	EntityManager* Instance;
}

EntityManager::EntityManager()
{
	
}
EntityManager::~EntityManager()
{
	
}

void EntityManager::Startup()
{
	Instance = new EntityManager();
}
void EntityManager::Shutdown()
{
	for(Entity* e : Instance->entities)
	{
		if(e) delete e;
		e = nullptr;
	}
	Instance->entities.clear();

	delete Instance;
}

void EntityManager::Update(float deltaTime)
{
	for(Entity* e : GetEntities())
		e->Update(deltaTime);
}

void EntityManager::AddEntity(Entity* entity)
{
	Instance->entities.push_back(entity);

	entity->Initialize();
}

std::vector<Entity*> EntityManager::GetEntities()
{
	return Instance->entities;
}