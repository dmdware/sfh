










#ifndef FONT_H
#define FONT_H

#include "../platform.h"

#define MAX_CHARS	256
#define CODE_POINTS	110000

class Glyph
{
public:
	int32_t pixel[2];
	int32_t texsize[2];
	int32_t offset[2];
	int32_t origsize[2];
	float texcoord[4];
	bool rend;

	Glyph()
	{
		rend = false;
		pixel[0] = 0;
		pixel[1] = 0;
		texsize[0] = 0;
		texsize[1] = 0;
		offset[0] = 0;
		offset[1] = 0;
		origsize[0] = 0;
		origsize[1] = 0;
		texcoord[0] = 0;
		texcoord[1] = 0;
		texcoord[2] = 0;
		texcoord[3] = 0;
	}
};

class BmpFont
{
public:
	uint32_t texindex;
	float width;    //image width
	float height;   //image height
	float gheight;  //glyph height
	Glyph glyph[CODE_POINTS];
	//TTF_Font* ttf;

	BmpFont()
	{
		//ttf = NULL;
		width = 0;
		height = 0;
		gheight = 0;
	}

	void close()
	{
		//if(ttf)
		//	TTF_CloseFont(ttf);
		//ttf = NULL;

		for(int32_t fi=0; fi<CODE_POINTS; ++fi)
			glyph[fi].rend = false;
	}
};

#define FONT_EUROSTILE32	0
#define FONT_MSUIGOTHIC16	1
#define FONT_SMALLFONTS8	2
#define FONT_GULIM32		3
#define FONT_EUROSTILE16	4
#define FONT_CALIBRILIGHT16	5
#define FONT_MSUIGOTHIC10	6
#define FONT_ARIAL10		7
#define FONT_TERMINAL10		8
#define FONT_SMALLFONTS10	9
//#define FONT_TERMINAL11		10
#define FONTS				10
extern BmpFont g_font[FONTS];

#define MAINFONT8 (FONT_TERMINAL10)
//#define MAINFONT8 (FONT_ARIAL10)
//#define MAINFONT8 (FONT_CALIBRILIGHT16)
//#define MAINFONT8 (FONT_TERMINAL10)
//#define MAINFONT8 (FONT_TERMINAL11)
//#define MAINFONT16 MAINFONT8
//#define MAINFONT16 (FONT_TERMINAL10)
//#define MAINFONT16 (FONT_MSUIGOTHIC16)
#define MAINFONT16 (FONT_CALIBRILIGHT16)
//#define MAINFONT16 (FONT_SMALLFONTS10)
//#define MAINFONT32 (FONT_TERMINAL10)
//#define MAINFONT32 (MAINFONT16)
#define MAINFONT32 (FONT_EUROSTILE32)
//#define MAINFONT32 (FONT_SMALLFONTS10)
//#define MAINFONT8 MAINFONT16

class RichText;

void LoadFonts();
void UseFontTex();
void UseIconTex(int32_t ico);
void DrawGlyph(float left, float top, float right, float bottom, float texleft, float textop, float texright, float texbottom);
void DrawGlyphF(float left, float top, float right, float bottom, float texleft, float textop, float texright, float texbottom);
void HighlGlyphF(float left, float top, float right, float bottom);
void DrawLine(int32_t fnt, float startx, float starty, const RichText* text, const float* color=NULL, int32_t caret=-1);
void DrawLineF(int32_t fnt, float startx, float starty,  float framex1, float framey1, float framex2, float framey2, const RichText* text, const float* color=NULL, int32_t caret=-1);
void DrawShadowedText(int32_t fnt, float startx, float starty, const RichText* text, const float* color=NULL, int32_t caret=-1);
void DrawShadowedTextF(int32_t fnt, float startx, float starty, float framex1, float framey1, float framex2, float framey2, const RichText* text, const float* color=NULL, int32_t caret=-1);
void DrawCenterShadText(int32_t fnt, float startx, float starty, const RichText* text, const float* color=NULL, int32_t caret=-1);
void DrawBoxShadText(int32_t fnt, float startx, float starty, float width, float height, const RichText* text, const float* color, int32_t ln, int32_t caret);
void DrawBoxShadTextF(int32_t fnt, float startx, float starty, float width, float height, const RichText* text, const float* color, int32_t ln, int32_t caret, float framex1, float framey1, float framex2, float framey2);
void DrawBoxTextF(int32_t fnt, float startx, float starty, float width, float height, const RichText* text, const float* color, int32_t ln, int32_t caret, float framex1, float framey1, float framex2, float framey2);
int32_t CountLines(const RichText* text, int32_t fnt, float startx, float starty, float width, float height);
int32_t GetLineStart(const RichText* text, int32_t fnt, float startx, float starty, float width, float height, int32_t getline);
int32_t EndX(const RichText* text, int32_t lastc, int32_t fnt, float startx, float starty);
int32_t MatchGlyphF(const RichText* text, int32_t fnt, int32_t matchx, float startx, float starty, float framex1, float framey1, float framex2, float framey2);
void HighlightF(int32_t fnt, float startx, float starty, float framex1, float framey1, float framex2, float framey2, const RichText* text, int32_t highlstarti, int32_t highlendi);
void NextLineBreak();
void AdvGlyph();
int32_t TextWidth(int32_t fnt, const RichText* text);

#endif
