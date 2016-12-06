










#ifndef VEC3I_H
#define VEC3I_H

#include "../platform.h"

class Matrix;

class Vec3i
{
public:
	int32_t x, y, z;

	Vec3i()
	{
		x = y = z = 0;
	}

	Vec3i(int32_t X, int32_t Y, int32_t Z)
	{
		x = X;
		y = Y;
		z = Z;
	}

	Vec3i(const int32_t* values)
	{
		set(values);
	}

	bool operator==(const Vec3i vVector) const
	{
		if(x == vVector.x && y == vVector.y && z == vVector.z)
			return true;

		return false;
	}

	bool operator!=(const Vec3i vVector) const
	{
		if(x == vVector.x && y == vVector.y && z == vVector.z)
			return false;

		return true;
	}

	Vec3i operator+(const Vec3i vVector) const
	{
		return Vec3i(vVector.x + x, vVector.y + y, vVector.z + z);
	}

	Vec3i operator-(const Vec3i vVector) const
	{
		return Vec3i(x - vVector.x, y - vVector.y, z - vVector.z);
	}

	Vec3i operator*(const int32_t num) const
	{
		return Vec3i(x * num, y * num, z * num);
	}

	Vec3i operator*(const Vec3i v) const
	{
		return Vec3i(x * v.x, y * v.y, z * v.z);
	}

	Vec3i operator/(const int32_t num) const
	{
		return Vec3i(x / num, y / num, z / num);
	}

	inline void set(const int32_t* values)
	{
		x = values[0];
		y = values[1];
		z = values[2];
	}

	void transform(const Matrix& m);
	void transform3(const Matrix& m);
};

bool Close(Vec3i a, Vec3i b);

#endif
