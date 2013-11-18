#pragma once

#include "Common.h"
#include "Unit.hpp"

namespace MicroSearch
{

class Unit;

namespace MoveTypes
{
	enum {NONE, ATTACK, RELOAD, MOVE, PASS, HEAL};
};

class Move 
{

public:

	IDType			_unit,
					_player,
					_moveType,
					_moveIndex;

	Move()
		: _unit(255)
		, _player(255)
		, _moveType(MoveTypes::NONE)
		, _moveIndex(255)
	{

	}

	Move( const IDType & unitIndex, const IDType & player, const IDType & type, const IDType & moveIndex)
		: _unit(unitIndex)
		, _player(player)
		, _moveType(type)
		, _moveIndex(moveIndex)
	{
		
	}

	const IDType & unit()	const	{ return _unit; }
	const IDType & player() const	{ return _player; }
	const IDType & type()	const	{ return _moveType; }
	const IDType & index()	const	{ return _moveIndex; }

	const std::string moveString() const
	{
		if (_moveType == MoveTypes::ATTACK) 
		{
			return "ATTACK";
		}
		else if (_moveType == MoveTypes::MOVE)
		{
			return "MOVE";
		}
		else if (_moveType == MoveTypes::RELOAD)
		{
			return "RELOAD";
		}
		else if (_moveType == MoveTypes::PASS)
		{
			return "PASS";
		}
		else if (_moveType == MoveTypes::HEAL)
		{
			return "HEAL";
		}

		return "NONE";
	}

	const Position getDir() const
	{
		assert(_moveType == MoveTypes::MOVE);

		return Position(Search::Constants::Move_Dir[_moveIndex][0], Search::Constants::Move_Dir[_moveIndex][1]);
	}
};
}