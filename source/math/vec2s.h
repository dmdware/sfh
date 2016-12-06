










#ifndef VEC2S_H
#define VEC2S_H

// byte-align structures
#pragma pack(push, 1)

class Vec2s
{
public:
	int16_t x, y;

	Vec2s()
	{
		x = y = 0;
	}

	Vec2s(int16_t X, int16_t Y)
	{
		x = X;
		y = Y;
	}

	Vec2s(const int16_t* values)
	{
		set(values);
	}

	bool operator==(const Vec2s vVector) const
	{
		if(x == vVector.x && y == vVector.y)
			return true;

		return false;
	}

	bool operator!=(const Vec2s vVector) const
	{
		if(x == vVector.x && y == vVector.y)
			return false;

		return true;
	}

	Vec2s operator+(const Vec2s vVector) const
	{
		return Vec2s(vVector.x + x, vVector.y + y);
	}

	Vec2s operator-(const Vec2s vVector) const
	{
		return Vec2s(x - vVector.x, y - vVector.y);
	}

	Vec2s operator*(const int16_t num) const
	{
		return Vec2s(x * num, y * num);
	}

	Vec2s operator*(const Vec2s v) const
	{
		return Vec2s(x * v.x, y * v.y);
	}

	Vec2s operator/(const int16_t num) const
	{
		return Vec2s(x / num, y / num);
	}

	inline void set(const int16_t* values)
	{
		x = values[0];
		y = values[1];
	}

	int16_t& operator[](unsigned char in)
	{
		return *(int16_t*)(((char*)this)+in);
	}
};

// Default alignment
#pragma pack(pop)

#endif
