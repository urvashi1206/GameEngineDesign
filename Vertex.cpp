#include "Vertex.h"

bool Vertex::operator==(const Vertex& other) const
{
    return position.x == other.position.x && position.y == other.position.y && position.z == other.position.z
        && normal.x == other.normal.x && normal.y == other.normal.y && normal.z == other.normal.z
        && tangent.x == other.tangent.x && tangent.y == other.tangent.y && tangent.z == other.tangent.z 
        && uv.x == other.uv.x && uv.y == other.uv.y;
}