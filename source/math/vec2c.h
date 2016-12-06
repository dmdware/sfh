










#ifndef VEC2C_H
#define VEC2C_H

class Vec2c
{
public:
	signed char x, y;

	Vec2c()
	{
		x = y = 0;
	}

	Vec2c(signed char X, signed char Y)
	{
		x = X;
		y = Y;
	}

	Vec2c(const signed char* values)
	{
		set(values);
	}

	bool operator==(const Vec2c vVector) const
	{
		if(x == vVector.x && y == vVector.y)
			return true;

		return false;
	}

	bool operator!=(const Vec2c vVector) const
	{
		if(x == vVector.x && y == vVector.y)
			return false;

		return true;
	}

	Vec2c operator+(const Vec2c vVector) const
	{
		return Vec2c(vVector.x + x, vVector.y + y);
	}

	Vec2c operator-(const Vec2c vVector) const
	{
		return Vec2c(x - vVector.x, y - vVector.y);
	}

	Vec2c operator*(const signed char num) const
	{
		return Vec2c(x * num, y * num);
	}

	Vec2c operator*(const Vec2c v) const
	{
		return Vec2c(x * v.x, y * v.y);
	}

	Vec2c operator/(const signed char num) const
	{
		return Vec2c(x / num, y / num);
	}

	inline void set(const signed char* values)
	{
		x = values[0];
		y = values[1];
	}
};

#endif
