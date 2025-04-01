#pragma once

#include "Entity.h"

class EntityManager
{
private:
	std::vector<Entity*> entities;

public:
	EntityManager();
	~EntityManager();

	static void Startup();
	static void Shutdown();

	static void Update(float deltaTime);

	static void AddEntity(Entity* entity);

	static std::vector<Entity*> GetEntities();
};