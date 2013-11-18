#pragma once

#include "Common.h"
#include "Math.h"

namespace MicroSearch
{
class Position
{
	// x,y location will be used for Units in a 'grid'
	PositionType		_x, 
						_y;

public:
	
	Position()
		: _x(0)
		, _y(0)
	{
	}

	Position(const PositionType & x, const PositionType & y)
		: _x(x)
		, _y(y)
	{
	}

	const bool operator < (const Position & rhs) const
	{
		if (x() == rhs.x())
		{
			return y() < rhs.y();
		}
		else
		{
			return x() < rhs.x();
		}
	}

	const Position operator + (const Position & rhs) const
	{
		return Position(x() + rhs.x(), y() + rhs.y());
	}

	const Position operator - (const Position & rhs) const
	{
		return Position(x() - rhs.x(), y() - rhs.y());
	}

	const Position scale(const float & f) const
	{
		return Position((PositionType)(f * x()), (PositionType)(f * y()));
	}

	void move(const Position & pos)
	{
		_x += pos.x();
		_y += pos.y();
	}
	
	void moveTo(const Position & pos)
	{
		_x = pos.x();
		_y = pos.y();
	}


	void move(const PositionType & x, const PositionType & y)
	{
		_x += x;
		_y += y;
	}

	void moveTo(const PositionType & x, const PositionType & y)
	{
		_x = x;
		_y = y;
	}

	const PositionType x() const
	{
		return _x;
	}

	const PositionType y() const
	{
		return _y;
	}

	inline void SSESqrt( float * pOut, float * pIn )
	{
	   _mm_store_ss( pOut, _mm_sqrt_ss( _mm_load_ss( pIn ) ) );
	   // compiles to movss, sqrtss, movss
	}

	const Position flipX() const
	{
		return Position(-_x,_y);
	}

	const Position flipY() const
	{
		return Position(_y,_x);
	}

	const Position flip() const
	{
		return Position(-_x, -_y);
	}

	const PositionType distSq(const Position & p) const	
	{ 
		return ((x() - p.x())*(x() - p.x()) + (y() - p.y())*(y() - p.y())); 
	}

	void print() const
	{
		printf("Position = (%d, %d)\n", _x, _y);
	}
};
}