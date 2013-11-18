#pragma once

#include "Common.h"
#include "Array.hpp"
#include "Unit.hpp"

#include <boost/foreach.hpp>

namespace MicroSearch
{

typedef std::vector< std::vector<bool> > bvv;

class Map
{
	size_t					_width;
	size_t					_height;
	size_t					_tileWidth;
	size_t					_tileHeight;
	bvv						_mapData;	// true if walkable



	bvv						_unitData;	// true if unit on tile
	bvv						_buildingData;

	const Position getWalkPosition(const Position & pixelPosition) const
	{
		return Position(pixelPosition.x() / 8, pixelPosition.y() / 8);
	}

public:

	Map() {}

	Map(BWAPI::Game * game) 
		: _width(game->mapWidth() * 4)
		, _height(game->mapHeight() * 4)
		, _tileWidth(game->mapWidth())
		, _tileHeight(game->mapHeight())
	{
		_mapData = bvv(_width, std::vector<bool>(_height, true));
		_unitData = bvv(_tileWidth, std::vector<bool>(_tileHeight, true));
		_buildingData = bvv(_tileWidth, std::vector<bool>(_tileHeight, true));

		for (size_t x(0); x<_width; ++x)
		{
			for (size_t y(0); y<_height; ++y)
			{
				setMapData(x, y, game->isWalkable(x, y));
			}
		}
	}
	
	const size_t & getWidth() const
	{
		return _width;
	}

	const size_t & getHeight() const
	{
		return _height;
	}

	const size_t & getTileWidth() const
	{
		return _tileWidth;
	}

	const size_t & getTileHeight() const
	{
		return _tileHeight;
	}

	const bool isWalkable(const Position & p) const
	{
		const Position & wp(getWalkPosition(p));

		return	(wp.x() > 0) &&	(wp.x() < (PositionType)_width) && 
				(wp.y() > 0) && (wp.y() < (PositionType)_height) &&
				getMapData(wp.x(), wp.y());
	}

	const bool isWalkable(const size_t & x, const size_t & y) const
	{
		return	x > 0 && x < (PositionType)_width && 
				y > 0 && y < (PositionType)_height &&
				getMapData(x, y);
	}

	const bool getMapData(const size_t & x, const size_t & y) const
	{
		return _mapData[x][y];
	}

	const bool getUnitData(const size_t & x, const size_t & y) const
	{
		return _unitData[x][y];
	}

	void setMapData(const size_t & x, const size_t & y, const bool val)
	{
		_mapData[x][y] = val;
	}

	void setUnitData(BWAPI::Game * game)
	{
		_unitData = bvv(getTileWidth(), std::vector<bool>(getTileHeight(), true));

		BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->getAllUnits())
		{
			if (!unit->getType().isBuilding())
			{
				addUnit(unit);
			}
		}
	}

	const bool canBuildHere(BWAPI::TilePosition pos)
	{
		return _unitData[pos.x()][pos.y()] && _buildingData[pos.x()][pos.y()];
	}

	void setBuildingData(BWAPI::Game * game)
	{
		_buildingData = bvv(getTileWidth(), std::vector<bool>(getTileHeight(), true));

		BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->getAllUnits())
		{
			if (unit->getType().isBuilding())
			{
				addUnit(unit);
			}
		}
	}

	void addUnit(BWAPI::Unit * unit)
	{
		BWAPI::Position pos = unit->getPosition();
		BWAPI::UnitType type = unit->getType();

		if (type.isBuilding())
		{
			int tx = pos.x() / TILE_SIZE;
			int ty = pos.y() / TILE_SIZE;
			int sx = type.tileWidth(); 
			int sy = type.tileHeight();
			for(int x = tx; x < tx + sx && x < (int)getTileWidth(); ++x)
			{
				for(int y = ty; y < ty + sy && y < (int)getTileHeight(); ++y)
				{
					_buildingData[x][y] = false;
				}
			}
		}
		else
		{
			int startX = (pos.x() - type.dimensionLeft()) / TILE_SIZE;
			int endX   = (pos.x() + type.dimensionRight() + TILE_SIZE - 1) / TILE_SIZE; // Division - round up
			int startY = (pos.y() - type.dimensionUp()) / TILE_SIZE;
			int endY   = (pos.y() + type.dimensionDown() + TILE_SIZE - 1) / TILE_SIZE;
			for (int x = startX; x < endX && x < (int)getTileWidth(); ++x)
			{
				for (int y = startY; y < endY && y < (int)getTileHeight(); ++y)
				{
					_unitData[x][y] = false;
				}
			}
		}
	}

	unsigned int * getRGBATexture()
	{
		unsigned int * data = new unsigned int[getWidth() * getHeight()];
		for (size_t x(0); x<getWidth(); ++x)
		{
			for (size_t y(0); y<getHeight(); ++y)
			{
				if (!isWalkable(x, y))
				{
					data[y*getWidth() + x] = 0xffffffff;
				}
				else
				{
					data[y*getWidth() + x] = 0x00000000;
				}
			}
		}

		return data;
	}

	void write(const std::string & filename)
	{
		std::ofstream fout(filename.c_str());
		fout << getWidth() << "\n" << getHeight() << "\n";

		for (size_t y(0); y<getHeight(); ++y)
		{
			for (size_t x(0); x<getWidth(); ++x)
			{
				fout << (isWalkable(x, y) ? 1 : 0);
			}

			fout << "\n";
		}

		fout.close();
	}

	void load(const std::string & filename)
	{
		std::ifstream fin(filename.c_str());
		std::string line;
		
		printf("Loading map..\n");

		getline(fin, line);

		_width = atoi(line.c_str());

		getline(fin, line);

		_height = atoi(line.c_str());

		_mapData = bvv(_width, std::vector<bool>(_height, true));
		_unitData = bvv(_width, std::vector<bool>(_height, true));

		for (size_t y(0); y<getHeight(); ++y)
		{
			getline(fin, line);

			for (size_t x(0); x<getWidth(); ++x)
			{
				_mapData[x][y] = line[x] == '1' ? true : false;
			}
		}

		fin.close();

		printf("Loading map complete..\n");
	}
};
}