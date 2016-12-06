










#ifndef VEC2I_H
#define VEC2I_H

#include "../platform.h"

class Vec2i
{
public:
	int32_t x, y;

	Vec2i()
	{
		x = y = 0;
	}

	Vec2i(int32_t X, int32_t Y)
	{
		x = X;
		y = Y;
	}

	Vec2i(const int32_t* values)
	{
		set(values);
	}

	bool operator==(const Vec2i vVector) const
	{
		if(x == vVector.x && y == vVector.y)
			return true;

		return false;
	}

	bool operator!=(const Vec2i vVector) const
	{
		if(x == vVector.x && y == vVector.y)
			return false;

		return true;
	}

	Vec2i operator+(const Vec2i vVector) const
	{
		return Vec2i(vVector.x + x, vVector.y + y);
	}

	Vec2i operator-(const Vec2i vVector) const
	{
		return Vec2i(x - vVector.x, y - vVector.y);
	}

	Vec2i operator*(const int32_t num) const
	{
		return Vec2i(x * num, y * num);
	}

	Vec2i operator*(const Vec2i v) const
	{
		return Vec2i(x * v.x, y * v.y);
	}

	Vec2i operator/(const int32_t num) const
	{
		return Vec2i(x / num, y / num);
	}

	inline void set(const int32_t* values)
	{
		x = values[0];
		y = values[1];
	}
};

#endif
