










#ifndef VEC2UC_H
#define VEC2UC_H

class Vec2uc
{
public:
	unsigned char x, y;

	Vec2uc()
	{
		x = y = 0;
	}

	Vec2uc(unsigned char X, unsigned char Y)
	{
		x = X;
		y = Y;
	}

	Vec2uc(const unsigned char* values)
	{
		set(values);
	}

	bool operator==(const Vec2uc vVector) const
	{
		if(x == vVector.x && y == vVector.y)
			return true;

		return false;
	}

	bool operator!=(const Vec2uc vVector) const
	{
		if(x == vVector.x && y == vVector.y)
			return false;

		return true;
	}

	Vec2uc operator+(const Vec2uc vVector) const
	{
		return Vec2uc(vVector.x + x, vVector.y + y);
	}

	Vec2uc operator-(const Vec2uc vVector) const
	{
		return Vec2uc(x - vVector.x, y - vVector.y);
	}

	Vec2uc operator*(const unsigned char num) const
	{
		return Vec2uc(x * num, y * num);
	}

	Vec2uc operator*(const Vec2uc v) const
	{
		return Vec2uc(x * v.x, y * v.y);
	}

	Vec2uc operator/(const unsigned char num) const
	{
		return Vec2uc(x / num, y / num);
	}

	inline void set(const unsigned char* values)
	{
		x = values[0];
		y = values[1];
	}
};

#endif
