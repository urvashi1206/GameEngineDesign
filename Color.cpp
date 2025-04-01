#include "Color.h"

Color::Color(float r, float g, float b, float a) : 
	r(r), g(g), b(b), a(a)
{

}
Color::~Color()
{

}

bool Color::operator==(const Color& other) const
{
	return r == other.r && g == other.g && b == other.b && a == other.a;
}