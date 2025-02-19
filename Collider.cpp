#include "Collider.h"

Collider::Collider() : colliderType(ColliderType::None)
{

}
Collider::Collider(Transform* transform) : colliderType(ColliderType::None), transform(transform)
{

}
Collider::~Collider()
{

}