#pragma once

#include "Common.h"

// type definitions for storing data
typedef		unsigned char		IDType;
typedef		unsigned char		UnitCountType;
typedef		unsigned char		ChildCountType;
typedef 	int					PositionType;
typedef 	int					TimeType;
typedef		short				HealthType;
typedef		int					ScoreType;
typedef		unsigned int		HashType;
typedef		long long unsigned	MoveTuple;
typedef		std::pair<TimeType,TimeType> AttackFrameData;

namespace MicroSearch
{
	
class AlphaBetaScore
{
	ScoreType	_val;
	int			_numMoves;

public:

	AlphaBetaScore()
		: _val(0)
		, _numMoves(0)
	{
	}

	AlphaBetaScore(const ScoreType & val, const int & numMoves)
		: _val(val)
		, _numMoves(numMoves)
	{
	}

	const bool operator < (const AlphaBetaScore & rhs) const
	{
		if (_val < rhs._val)
		{
			return true;
		}
		else if (_val == rhs._val)
		{
			return _numMoves > rhs._numMoves;
		}
		else
		{
			return false;
		}
	}

	const bool operator > (const AlphaBetaScore & rhs) const
	{
		if (_val > rhs._val)
		{
			return true;
		}
		else if (_val == rhs._val)
		{
			return _numMoves < rhs._numMoves;
		}
		else
		{
			return false;
		}
	}

	const bool operator <= (const AlphaBetaScore & rhs) const
	{
		if (_val > rhs._val)
		{
			return true;
		}
		else if (_val == rhs._val)
		{
			return _numMoves >= rhs._numMoves;
		}
		else
		{
			return false;
		}
	}

	const bool operator >= (const AlphaBetaScore & rhs) const
	{
		if (_val > rhs._val)
		{
			return true;
		}
		else if (_val == rhs._val)
		{
			return _numMoves <= rhs._numMoves;
		}
		else
		{
			return false;
		}
	}

	const bool operator == (const AlphaBetaScore & rhs) const
	{
		return (_val == rhs._val) && (_numMoves == rhs._numMoves);
	}

	const ScoreType & val() const { return _val; }
	const TimeType & numMoves() const { return _numMoves; }
};


class AlphaBetaMove
{
	MoveTuple _move;
	bool _isValid;

public:

	AlphaBetaMove()
		: _move(0)
		, _isValid(false)
	{
	}

	AlphaBetaMove(const MoveTuple & move, const bool & isValid)
		: _move(move)
		, _isValid(isValid)
	{
	}

	const bool isValid() const { return _isValid; }
	const MoveTuple & moveTuple() const { return _move; }
};

class TTBestMove
{
	AlphaBetaMove _firstMove;
	AlphaBetaMove _secondMove;

public:

	TTBestMove()
	{
	}

	TTBestMove(const AlphaBetaMove & first, const AlphaBetaMove & second)
		: _firstMove(first)
		, _secondMove(second)
	{
	}

	const AlphaBetaMove & firstMove() const		{ return _firstMove; }
	const AlphaBetaMove & secondMove() const	{ return _secondMove; }
};


class AlphaBetaValue
{	
	AlphaBetaScore	_score;
	AlphaBetaMove	_move;

public:

	AlphaBetaValue()
	{
	}

	AlphaBetaValue(const AlphaBetaScore & score, const AlphaBetaMove & abMove)
		: _score(score)
		, _move(abMove)
	{
	}

	const AlphaBetaScore & score() const { return _score; }
	const AlphaBetaMove & abMove() const { return _move; }
};

}
