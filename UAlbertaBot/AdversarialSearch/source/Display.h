#ifdef USING_VISUALIZATION_LIBRARIES

#pragma once

#include "Common.h"
#include "GameState.h"

#include "BWAPI.h"
#include "boost/foreach.hpp"

#include <SDL.h>
#include <SDL_opengl.h>

#include "SearchResults.hpp"
#include "MicroSearchParameters.h"

#include <sys/stat.h>

//#include <ftgl.h>
#include <FTGLPixmapFont.h>
namespace MicroSearch
{
struct float3
{
	float x,y,z;

	float3() {}
	float3(float f) : x(f), y(f), z(f) {}
	float3(float x, float y, float z) : x(x), y(y), z(z) {}

	operator const float * () const { return &x; }
	float3 operator + (const float3 & v) const { return float3(x+v.x,y+v.y,z+v.z); }
	float3 operator - (const float3 & v) const { return float3(x-v.x,y-v.y,z-v.z); }
	float3 operator * (const float3 & v) const { return float3(x*v.x,y*v.y,z*v.z); }
	float3 operator / (const float3 & v) const { return float3(x/v.x,y/v.y,z/v.z); }

	operator float * () { return &x; }
	const float3 & operator += (const float3 & v) { x+=v.x; y+=v.y; z+=v.z; return *this; }
	const float3 & operator -= (const float3 & v) { x-=v.x; y-=v.y; z-=v.z; return *this; }
	const float3 & operator *= (const float3 & v) { x*=v.x; y*=v.y; z*=v.z; return *this; }
	const float3 & operator /= (const float3 & v) { x/=v.x; y/=v.y; z/=v.z; return *this; }
};

struct int2
{
	int x,y;

	int2() {}
	int2(int x, int y) : x(x), y(y) {}

	operator const int * () const { return &x; }
};

struct Shape
{
	std::vector<int2>	points;
	float3				color;
	bool				solid;
public:
	Shape(const float3 & color, bool solid) : color(color), solid(solid) {}

	void AddPoint(const int2 & point) { points.push_back(point); }

	void OnRender() const;
};

struct TextElement
{
	int2		position;
	float3		color;
	std::string	text;

	TextElement() {}
	TextElement(const int2 & position, const float3 & color, const char * text) : position(position), color(color), text(text) {}
};

class Display
{
	unsigned int texFont;

	int	windowSizeX;
	int windowSizeY;

	int zoomX;
	int zoomY;

	int cameraX;
	int cameraY;

	int mapWidth;
	int mapHeight;

	int mapPixelWidth;
	int mapPixelHeight;

	bool bl,br,bd,bu;

	std::vector<Position>		textureSizes;
	std::vector<Shape>			shapes;
	std::vector<TextElement>	textElements;

	FTGLPixmapFont font;
	SDL_Surface * screen;

	GameState state;
	SearchResults results;
	MicroSearchParameters params;

	IDType playerTypes[2];

	bool started;
	bool drawResults;

	void HandleEvents();

	void RenderMainMap();
	void RenderShapes();
	void RenderMinimap();
	void RenderTextOverlay();
	void RenderInformation();
	
	const std::string getTextureFileName(const BWAPI::UnitType type) const;

	void LoadTextures();
	void LoadTexture(int textureNumber, const char * fileName);

	void DrawSearchResults(int x, int y);
	void DrawParameters(int x, int y);
	
	void DrawCircle(float cx, float cy, float r, int num_segments) const;
	void DrawText(const int & x, const int & y, const int & size, const std::string & text);

	void DrawUnitTexture(const Unit & unit) const;
	void DrawUnitMove(const Unit & unit, const float3 & color) const;
	void DrawUnitHP(const Unit & unit, const float3 & color) const;

	void RenderTerrain(int wx0, int wy0, int wx1, int wy1);
	void RenderUnits();
	void RenderUnit(const MicroSearch::Unit & unit);

	float3 GetWalkTileColor(int x, int y) const;


public:

	Display(const int mw, const int mh);
	~Display();

	void OnStart();
	void OnFrame();

	
	void LoadMapTexture(MicroSearch::Map * map, int textureNumber);
	void SetState(const GameState & s);
	void SetResults(const SearchResults & r);
	void SetParams(const MicroSearchParameters & p);
	void SetPlayerTypes(const IDType & p1, const IDType & p2);

	void AddShape(const Shape & shape) { shapes.push_back(shape); }
	void AddTextElement(const TextElement & element) { textElements.push_back(element); }
};
}

#endif