#pragma once

#include <algorithm>

struct Color
{
public:
	float r, g, b, a;

public:
	Color(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);
	~Color();

	bool operator==(const Color& other) const;
};

namespace std
{
	template<>
	struct hash<Color>
	{
		std::size_t operator()(const Color& color) const
		{
			/* Hash and combine r, g, b, a */

			std::hash<float> hasher;

			return hasher(color.r)
				^ (hasher(color.g) << 1)
				^ (hasher(color.b) << 2)
				^ (hasher(color.a) << 3);
		}
	};
}