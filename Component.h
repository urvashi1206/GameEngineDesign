#pragma once

#include "Entity.h"

class Component
{
protected:
	Entity* entity;

public:
	virtual ~Component() = default;

protected:
	virtual void Initialize() = 0;
	virtual void Update(float deltaTime) = 0;

	virtual void SetEntity(Entity* entity);

	friend class Entity;
};