











#ifndef VERTEXARRAY_H
#define VERTEXARRAY_H

#include "../platform.h"
#include "../math/vec3f.h"
#include "../math/vec2f.h"

#define VBO_POSITION		0
#define VBO_TEXCOORD		1
#define VBO_NORMAL			2
#define VBOS				3

class VertexArray
{
public:
	int32_t numverts;
	Vec3f* vertices;
	Vec2f* texcoords;
	Vec3f* normals;
	//Vec3f* tangents;
	uint32_t vbo[VBOS];

	VertexArray(const VertexArray& original);
	VertexArray& operator=(VertexArray const &original);
	VertexArray()
	{
		numverts = 0;
		for(int32_t i=0; i<VBOS; i++)
			vbo[i] = -1;
	}

	~VertexArray()
	{
		free();
	}

	void genvbo();
	void delvbo();
	void alloc(int32_t numv);
	void free();
};



void CopyVA(VertexArray* to, const VertexArray* from);
void CopyVAs(VertexArray** toframes, int32_t* tonframes, VertexArray* const* fromframes, int32_t fromnframes);

#endif
