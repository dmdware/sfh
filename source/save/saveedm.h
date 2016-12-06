


#include "../platform.h"
#include "../math/vec3f.h"
#include "../math/polygon.h"
#include "../math/triangle.h"
#include "../bsp/brush.h"

class Map
{
public:
	int m_nbrush;
	Brush* m_brush;
	std::list<int> m_transpbrush;
	std::list<int> m_opaquebrush;
	std::list<int> m_skybrush;

	Map();
	~Map();
	void destroy();
};

extern Map g_map;

void DrawMap(Map* map);
void DrawMap2(Map* map);

#define EDMAP_VERSION		2.0f

#define TAG_EDMAP		{'D', 'M', 'D', 'S', 'P'}	//DMD sprite project

class EdMap;
class TexRef;
class Brush;

class Map;
class CutBrush;
class Brush;

void ReadBrush(FILE* fp, TexRef* texrefs, Brush* b);
void SaveBrush(FILE* fp, int* texrefs, Brush* b);
void SaveEdMap(const char* fullpath, EdMap* map);
bool LoadEdMap(const char* fullpath, EdMap* map);
void FreeEdMap(EdMap* map);

#define TAG_MAP		{'D', 'M', 'D', 'M', 'C'}
//#define MAP_VERSION		1.0f

void SaveTexs(FILE* fp, int* texrefs, std::list<Brush>& brushes);
void SaveMap(const char* fullpath, std::list<Brush>& brushes);
bool LoadMap(const char* fullpath, Map* map);