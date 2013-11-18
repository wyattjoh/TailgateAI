#pragma once

#include "Common.h"

#include "BWAPI.h"
#include "Position.hpp"
#include "Move.hpp"
#include "Hash.h"
#include <iostream>

namespace MicroSearch
{

class Unit 
{
	BWAPI::UnitType		_unitType;				// the BWAPI unit type that we are mimicing
	
	Position			_position;				// current location in a possibly infinite space
	
	IDType				_unitID,				// unique unit ID to the state it's contained in
						_playerID;				// the player who controls the unit
	
	HealthType			_currentHP;				// current HP of the unit
	HealthType			_currentEnergy;

	TimeType			_timeCanMove,			// time the unit can next move
						_timeCanAttack;			// time the unit can next attack

	Move				_previousMove;			// the previous move that the unit performed
	TimeType			_previousMoveTime;		// the time the previous move was performed
	Position			_previousPosition;

public:

	// default constructor
	Unit()
		: _unitType(BWAPI::UnitTypes::None)
		, _unitID(255)
		, _playerID(255)
		, _currentHP(0)
		, _currentEnergy(0)
		, _timeCanMove(0)
		, _timeCanAttack(0)
		, _previousMoveTime(0)
	{
	}
	
	// test constructor for setting all variables of a unit
	Unit(const BWAPI::UnitType unitType, const Position & pos, const IDType & unitID, const IDType & playerID, 
		 const HealthType & hp, const HealthType & energy, const TimeType & tm, const TimeType & ta) 
		: _unitType(unitType)
		, _position(pos)
		, _unitID(unitID)
		, _playerID(playerID)
		, _currentHP(hp)
		, _currentEnergy(energy)
		, _timeCanMove(tm)
		, _timeCanAttack(ta)
		, _previousMoveTime(0)
	{
	}

	// constructor based on a BWAPI unit, used by UAlbertaBot
	Unit(BWAPI::Unit * unit, const IDType & playerID, const TimeType & gameTime)
		: _unitType(unit->getType() == BWAPI::UnitTypes::Terran_Medic ? BWAPI::UnitTypes::Terran_Marine : unit->getType())
		, _position(Position(unit->getPosition().x(), unit->getPosition().y()))
		, _unitID((IDType)unit->getID())
		, _playerID(playerID)
		, _currentHP((HealthType)(unit->getHitPoints() + unit->getShields()))
		, _currentEnergy((HealthType)unit->getEnergy())
		, _timeCanMove((TimeType)(BWAPI::Broodwar->getFrameCount()))
		, _timeCanAttack((TimeType)(BWAPI::Broodwar->getFrameCount() + unit->getGroundWeaponCooldown() + unit->getAirWeaponCooldown()))
		, _previousMoveTime(gameTime)
	{
	}

	// constructor for units to construct basic units, sets some things automatically
	Unit(const BWAPI::UnitType unitType, const IDType & unitID, const IDType & playerID, const Position & pos) 
		: _unitType(unitType)
		, _position(pos)
		, _unitID(unitID)
		, _playerID(playerID)
		, _currentHP(maxHP())
		, _currentEnergy(unitType == BWAPI::UnitTypes::Terran_Medic ? Search::Constants::Starting_Energy : 0)
		, _timeCanMove(0)
		, _timeCanAttack(0)
		, _previousMoveTime(0)
	{
	}

	// Less than operator, used for sorting the GameState unit array.
	// Units are sorted in this order:
	//		1) alive < dead
	//		2) firstTimeFree()
	//		3) currentHP()
	//		4) pos()
	const bool operator < (const Unit & rhs) const
	{
		if (!isAlive())
		{
			return false;
		}
		else if (!rhs.isAlive())
		{
			return true;
		}

		if (firstTimeFree() == rhs.firstTimeFree())
		{
			if (currentHP() == rhs.currentHP())
			{
				return pos() < rhs.pos();
			}
			else
			{
				return currentHP() < rhs.currentHP();
			}
		}
		else
		{
			return firstTimeFree() < rhs.firstTimeFree();
		}
	}

	// Less than operator, used for sorting the GameState unit array.
	// Units are sorted in this order:
	//		1) alive < dead
	//		2) firstTimeFree()
	//		3) currentHP()
	//		4) pos()
	const bool operator < (Unit * rhs) const
	{
		if (!isAlive())
		{
			return false;
		}
		else if (!rhs->isAlive())
		{
			return true;
		}

		if (firstTimeFree() == rhs->firstTimeFree())
		{
			if (currentHP() == rhs->currentHP())
			{
				return pos() < rhs->pos();
			}
			else
			{
				return currentHP() < rhs->currentHP();
			}
		}
		else
		{
			return firstTimeFree() < rhs->firstTimeFree();
		}
	}

	// compares a unit based on unit id
	const bool equalsID(const Unit & rhs) const
	{ 
		return _unitID == rhs._unitID; 
	}

	// returns whether or not this unit can attack a given unit at a given time
	const bool canAttackTarget(const Unit & unit, const TimeType & gameTime) const
	{
		if (unit.type().isFlyer())
		{
			if (_unitType.airWeapon() == BWAPI::WeaponTypes::None)
			{
				return false;
			}
		}
		else
		{
			if (_unitType.groundWeapon() == BWAPI::WeaponTypes::None)
			{
				return false;
			}
		}

		// range of this unit attacking
		PositionType r = range();

		// return whether the target unit is in range
		return (r * r) >= distSq(unit, gameTime);
	}

	const bool canHealTarget(const Unit & unit, const TimeType & gameTime) const
	{
		// if the unit can't heal or the target unit is not on the same team
		if (!canHeal() || !unit.isOrganic() || !(unit.player() == player()) || (unit.currentHP() == unit.maxHP()))
		{
			// then it can't heal the target
			return false;
		}
		
		// range of this unit attacking
		PositionType r = healRange();

		// return whether the target unit is in range
		return (r * r) >= distSq(unit, gameTime);
	}
	
	const Position & position() const
	{
		return _position;
	}

	// take an attack, subtract the hp
	void takeAttack(const Unit & attacker)
	{
		updateCurrentHP(_currentHP - attacker.damage());
	}

	void takeHeal(const Unit & healer)
	{
		updateCurrentHP(_currentHP + healer.healAmount());
	}

	// returns whether or not this unit is alive
	const bool isAlive() const
	{
		return _currentHP > 0;
	}

	// attack a unit, set the times accordingly
	void attack(const Move & move, const Unit & target, const TimeType & gameTime)
	{
		//printf("I am attacking at frame %d\n", gameTime);

		// can't attack again until attack cooldown is up
		updateAttackTime(gameTime + attackCooldown());

		// can't move again until the attack animation has finished
		// if this is a repeat attack
		if (_previousMove.type() == MoveTypes::ATTACK || _previousMove.type() == MoveTypes::RELOAD)
		{
			// add the repeat attack animation duration
			updateMoveTime(gameTime + attackRepeatFrameTime());
		}
		// otherwise it's an initial attack
		else
		{
			// add the initial attack animation duration
			updateMoveTime(gameTime + attackInitFrameTime());
		}

		//printf("I can move again at time %d\n", timeMove());
		setPreviousMove(move, gameTime);
	}

	// attack a unit, set the times accordingly
	void heal(const Move & move, const Unit & target, const TimeType & gameTime)
	{
		_currentEnergy -= healCost();

		// can't attack again until attack cooldown is up
		updateAttackTime(gameTime + healCooldown());
		updateMoveTime(gameTime + healCooldown());

		if (currentEnergy() < healCost())
		{
			updateAttackTime(1000000);
		}
		
		setPreviousMove(move, gameTime);
	}

	// unit update for moving based on a given Move
	void move(const Move & move, const TimeType & gameTime) 
	{
		_previousPosition = pos();
		Position dir(Search::Constants::Move_Dir[move._moveIndex][0], Search::Constants::Move_Dir[move._moveIndex][1]);

		// if we can't attack, move until we can
		if (!canHeal() && !canAttackNow())
		{
			moveUntilAttack(dir, gameTime);
		}
		// otherwise move the full distance
		else
		{
			// update the next time we can move
			updateMoveTime(gameTime + moveCooldown() + 1);

			// assume we need 2 frames to turn around after moving
			updateAttackTime(gameTime + moveCooldown() + 1);

			// update the position
			updatePosition(Search::Constants::Move_Distance * dir.x(), Search::Constants::Move_Distance * dir.y());
		}

		setPreviousMove(move, gameTime);
	}

	// unit is commanded to wait until his cooldown is up
	void waitUntilAttack(const Move & move, const TimeType & gameTime)
	{
		// do nothing until we can attack again
		updateMoveTime(_timeCanAttack);
		setPreviousMove(move, gameTime);
	}

	void pass(const Move & move, const TimeType & gameTime)
	{
		updateMoveTime(gameTime + Search::Constants::Pass_Move_Duration);
		updateAttackTime(gameTime + Search::Constants::Pass_Move_Duration);
		setPreviousMove(move, gameTime);
	}

	// moves the unit in direction dir until the attack cooldown is up
	void moveUntilAttack(const Position & dir, const TimeType & gameTime)
	{
		double dist = (_timeCanAttack - gameTime) * speed();

		// move to the location determined by how long we have until we can attack
		updatePosition((PositionType)(dist * dir.x()), (PositionType)(dist * dir.y()));

		// no point in moving again until we get there
		updateMoveTime(_timeCanAttack + 1);

		// assume worst case that we need 2 frames to turn around after moving
		updateAttackTime(_timeCanAttack + 1);
	}

	// return the BWAPI unit type name of this unit
	const std::string name() const
	{
		return _unitType.getName();
	}

	// returns the time needed by this unit to travel a given distance
	const TimeType moveCooldown(const PositionType & dist) const 
	{ 
		return (TimeType)(dist / _unitType.topSpeed());
	}

	// returns the time needed by this unit to travel from its Position to a given Position
	const TimeType moveCooldown(const Position & p) const 
	{ 
		float dist;
		float distSq = static_cast<float>(((p.x()-x())*(p.x()-x())) + ((p.y()-y())*(p.y()-y())));

		// fast square root, defined in Common.h
		FAST_SQRT( &dist, &distSq );

		return static_cast<TimeType>((dist / _unitType.topSpeed())); 
	}

	const Position getMoveUntilAttackDest(const Position & dir, const TimeType & gameTime) const
	{
		double dist = (_timeCanAttack - gameTime) * speed();
		
		Position dP((PositionType)(dist * dir.x()), (PositionType)(dist * dir.y()));

		return pos() + dP;
	}

	// returns current position based on game time
	const Position currentPosition(const TimeType & gameTime) const
	{
		// if the previous move was MOVE, then we need to calculate where the unit is now
		if (_previousMove.type() == MoveTypes::MOVE)
		{
			// if gameTime is equal to previous move time then we haven't moved yet
			if (gameTime == _previousMoveTime)
			{
				return _previousPosition;
			}
			// else if game time is >= time we can move, then we have arrived at the destination
			else if (gameTime >= _timeCanMove)
			{
				return _position;
			}
			// otherwise we are still moving, so calculate the current position
			else
			{
				// the total duration of the current move happening
				TimeType moveDuration = _timeCanMove - _previousMoveTime;
				
				// the ratio of the time that has passed for this movement
				float moveTimeRatio = (float)(gameTime - _previousMoveTime) / moveDuration;
				
				// the destination for the previous move is the current position
				return _previousPosition + (_position - _previousPosition).scale(moveTimeRatio);
			}
		}
		// if it wasn't a MOVE, then we just return the Unit position
		else
		{
			return _position;
		}
	}

	// calculates the hash of this unit based on a given game time
	const HashType calculateHash(const size_t & hashNum, const TimeType & gameTime) const
	{
		Position currentPos = currentPosition(gameTime);

		return	  Hash::values[hashNum].positionHash(_playerID, currentPos.x(), currentPos.y()) 
				^ Hash::values[hashNum].getAttackHash(_playerID, timeAttack() - gameTime) 
				^ Hash::values[hashNum].getMoveHash(_playerID, timeMove() - gameTime)
				^ Hash::values[hashNum].getCurrentHPHash(_playerID, currentHP())
				^ Hash::values[hashNum].getUnitTypeHash(_playerID, typeID());
	}

	// calculates the hash of this unit based on a given game time, and prints debug info
	void debugHash(const size_t & hashNum, const TimeType & gameTime) const
	{
		std::cout << " Pos   " << Hash::values[hashNum].positionHash(_playerID, position().x(), position().y());
		std::cout << " Att   " << Hash::values[hashNum].getAttackHash(_playerID, timeAttack() - gameTime);
		std::cout << " Mov   " << Hash::values[hashNum].getMoveHash(_playerID, timeMove() - gameTime);
		std::cout << " HP    " << Hash::values[hashNum].getCurrentHPHash(_playerID, currentHP());
		std::cout << " Typ   " << Hash::values[hashNum].getUnitTypeHash(_playerID, typeID()) << "\n";;

		HashType hash = Hash::values[hashNum].positionHash(_playerID, position().x(), position().y()); std::cout << hash << "\n";
		hash ^= Hash::values[hashNum].getAttackHash(_playerID, timeAttack() - gameTime) ; std::cout << hash << "\n";
		hash ^= Hash::values[hashNum].getMoveHash(_playerID, timeMove() - gameTime); std::cout << hash << "\n";
		hash ^= Hash::values[hashNum].getCurrentHPHash(_playerID, currentHP()); std::cout << hash << "\n";
		hash ^= Hash::values[hashNum].getUnitTypeHash(_playerID, typeID()); std::cout << hash << "\n";
	}

	// returns the damage a unit does
	const HealthType damage() const	
	{ 
		return _unitType == BWAPI::UnitTypes::Protoss_Zealot ? 
							2 * (HealthType)_unitType.groundWeapon().damageAmount() : 
							    (HealthType)_unitType.groundWeapon().damageAmount(); 
	}

	const HealthType healAmount() const
	{
		return canHeal() ? 6 : 0;
	}

	// prints some information about this unit for debugging purposes
	void print() const { printf("%s %5d [%5d %5d] (%5d, %5d)\n", _unitType.getName().c_str(), currentHP(), timeAttack(), timeMove(), x(), y()); }

	// setters
	void updateCurrentHP(const HealthType & newHP)							{ _currentHP = std::min(maxHP(), newHP); }
	void updateAttackTime(const TimeType & newTime)							{ _timeCanAttack = newTime; }
	void updateMoveTime(const TimeType & newTime)							{ _timeCanMove = newTime; }
	void setCooldown(TimeType attack, TimeType move)						{ _timeCanAttack = attack; _timeCanMove = move; }
	void updatePosition(const PositionType & x, const PositionType & y)		{ _position.move(x, y); }
	void setUnitID(const IDType & id)										{ _unitID = id; }
	
	void setPreviousMove(const Move & m, const TimeType & previousMoveTime) 
	{	
		// if it was an attack move, store the unitID of the opponent unit
		_previousMove = m;
		_previousMoveTime = previousMoveTime; 
	}

	// getters
	const bool			canAttackNow()				const	{ return !canHeal() && _timeCanAttack <= _timeCanMove; }
	const bool			canMoveNow()				const	{ return isMobile() && _timeCanMove <= _timeCanAttack; }
	const bool			canHealNow()				const	{ return canHeal() && (currentEnergy() >= healCost()) && (_timeCanAttack <= _timeCanMove); }
	const bool			canKite()					const	{ return _timeCanMove < _timeCanAttack; }
	const bool			isMobile()					const	{ return _unitType.canMove(); }
	const bool			canHeal()					const	{ return _unitType == BWAPI::UnitTypes::Terran_Medic; }
	const bool			isOrganic()					const	{ return _unitType.isOrganic(); }
	const Position		pos()						const	{ return _position; }
	const PositionType	x()							const	{ return _position.x(); }
	const PositionType	y()							const	{ return _position.y(); }
	const PositionType	range()						const	{ return _unitType.groundWeapon().maxRange() + 32; }
	const PositionType	healRange()					const	{ return canHeal() ? 96 : 0; }
	const IDType		ID()						const	{ return _unitID; }
	const IDType		player()					const	{ return _playerID; }
	const HealthType	maxHP()						const	{ return (HealthType)_unitType.maxHitPoints() + (HealthType)_unitType.maxShields(); }
	const HealthType	currentHP()					const	{ return (HealthType)_currentHP; }
	const HealthType	currentEnergy()				const	{ return (HealthType)_currentEnergy; }
	const HealthType	maxEnergy()					const	{ return (HealthType)_unitType.maxEnergy(); }
	const HealthType	healCost()					const	{ return 3; }
	const TimeType		moveCooldown()				const	{ assert(isMobile()); return (TimeType)((double)Search::Constants::Move_Distance / _unitType.topSpeed()); }
	const TimeType		attackCooldown()			const	{ return (TimeType)_unitType.groundWeapon().damageCooldown(); }
	const TimeType		healCooldown()				const	{ return (TimeType)8; }
	const TimeType		timeAttack()				const	{ return _timeCanAttack; }
	const TimeType		timeMove()					const	{ return _timeCanMove; }
	const TimeType		previousMoveTime()			const	{ return _previousMoveTime; }
	const TimeType		firstTimeFree()				const	{ return _timeCanAttack <= _timeCanMove ? _timeCanAttack : _timeCanMove; }
	const TimeType 		attackInitFrameTime()		const	{ return Search::StarcraftData::getAttackFrames(_unitType).first; }
	const TimeType 		attackRepeatFrameTime()		const	{ return Search::StarcraftData::getAttackFrames(_unitType).second; }
	const int			typeID()					const	{ return _unitType.getID(); }
	const double		speed()						const	{ return _unitType.topSpeed(); }
	const BWAPI::UnitType type()					const	{ return _unitType; }
	const Move &		previousMove()				const	{ return _previousMove; }
	const float			dpf()						const	{ return (float)std::max(Search::Constants::Min_Unit_DPF, (float)damage() / ((float)attackCooldown() + 1)); }

	const PositionType distSq(const Unit & u, const TimeType & gameTime) const 
	{ 
		return distSq(u.currentPosition(gameTime), gameTime); 
	}

	const PositionType distSq(const Position & p, const TimeType & gameTime) const	
	{ 
		Position currentPos(currentPosition(gameTime));
		PositionType dX(currentPos.x() - p.x());
		PositionType dY(currentPos.y() - p.y());

		return dX*dX + dY*dY; 
	}

	
};

class UnitPtrCompare
{
public:
	const bool operator() (Unit * u1, Unit * u2) const
	{
		return *u1 < *u2;
	}
};
}