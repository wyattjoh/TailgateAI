#pragma once

#include "Common.h"
#include "Unit.hpp"
#include <algorithm>
#include "GraphViz.hpp"
#include "Array.hpp"
#include "MoveArray.hpp"
#include "Hash.h"
#include "MicroSearchParameters.h"
#include "Map.hpp"
#include <math.h>

typedef boost::shared_ptr<MicroSearch::Map> MapPtr;

namespace MicroSearch
{
class GameState 
{
	GameState *																		_parent;
	Map *																			_map;

	Array2D<Unit,   Search::Constants::Num_Players, Search::Constants::Max_Units>	_units;
	Array2D<Unit *, Search::Constants::Num_Players, Search::Constants::Max_Units>	_unitPtrs;
	Array<Unit, 1>																	_neutralUnits;

	Array<UnitCountType, Search::Constants::Num_Players>							_numUnits;
	Array<UnitCountType, Search::Constants::Num_Players>							_prevNumUnits;

	Array<float, Search::Constants::Num_Players>									_totalSumHP;
	Array<float, Search::Constants::Num_Players>									_totalSumSQRT;

	Array<int, Search::Constants::Num_Players>										_numMovements;
	
	TimeType																		_currentTime;
	size_t																			_maxUnits;

public:

	GameState();
	GameState(const GameState & state);
	GameState & operator = (const GameState & rhs);

	// data retrieval
	
	const int				winner()																const;
	const bool				playerDead(const IDType & player)										const;
	const bool				playersSeparated()														const;
	const bool				isWalkable(const Position & pos)										const;
	const bool				bothCanMove()															const;
	const bool				unitPtrCompare(Unit * u1, Unit * u2)									const;
	const bool				isDeadlock()															const;
	const size_t			numUnits(const IDType & player)											const;
	const size_t			numNeutralUnits()														const;
	const size_t			getMaxUnits()															const;
	const size_t			closestEnemyUnitDistance(const Unit & unit)								const;
	const size_t			getIndexOfUnitPtr(const size_t & player, const size_t & unit)			const;
	const Move &			moveThatGenerated()														const;
	const ChildCountType	moveIdentifier()														const;
	const TimeType			getTime()																const;
	const AlphaBetaScore	eval(const IDType & player, const IDType & evalMethod, 
								 const IDType simMethod = Search::PlayerModels::AttackClosest)		const;
	const ScoreType			evalSumHP(const IDType & player)										const;
	const ScoreType			evalSumDPS(const IDType & player)										const;
	const ScoreType			sumHP(const IDType & player)											const;
	const ScoreType			sumDPS(const IDType & player)											const;
	const AlphaBetaScore	evalSim(const IDType & player, const IDType & method)					const;
	const IDType			getEnemy(const IDType & player)											const;
	const HashType			calculateHash(const size_t & hashNum)									const;
	const Unit &			getUnit(const IDType & player, const UnitCountType & unit)				const;
	const Unit &			getUnitByID(const IDType & unitID)										const;
	const Unit &			getUnitByID(const IDType & player, const IDType & unitID)				const;
	const Unit &			getClosestEnemyUnit(const IDType & player, const IDType & unitIndex);
	const Unit &			getClosestOurUnit(const IDType & player, const IDType & unitIndex);
	const Unit &			getUnitDirect(const IDType & player, const IDType & unit)				const;
	const Unit &			getNeutralUnit(const size_t & u)										const;
	const float &			getTotalSumHP(const IDType & player)									const;
	const float &			getTotalSumDPS(const IDType & player)									const;

	const int &				getNumMovements(const IDType & player)									const;

	const IDType			whoCanMove()															const;

		  GameState *		getParent()																const;
		  Unit &			getUnit(const IDType & player, const UnitCountType & unit);

	const IDType			getUnitIndex(const IDType & player, const Unit & unit)					const;

	Map *					getMap()																const;

	bool matches(const GameState & s) const;

	// setters
	void sortUnits();
	void addNeutralUnit(const Unit & unit);
	void setMap(Map * map);
	void calculateStartingHealth();
	void setParent(GameState * parent);
	void setMaxPlayer(const IDType & player);
	void finishedMoving(bool normalize = true);
	void normalizeUnitPositions();
	void makeMove(const Move & theMove, GameState & parent);
	void setMoveThatGenerated(const ChildCountType & m, const Move & move);
	void addUnit(const size_t & num, const BWAPI::UnitType unitType, const IDType & playerID, const Position & pos);
	void addUnitSymmetric(const size_t & num, const BWAPI::UnitType unitType, const IDType & playerID, const Position & pos);
	void addUnit(const Unit & u);
	void generateMoves(MoveArray & moves, const IDType & playerIndex) const;
	void updateGameTime();
	void setTotalSumHP(const float & p1, const float & p2);
	void setTotalSumDPS(const float & p1, const float & p2);
	void setTime(const TimeType & time);
	void setMaxUnits(const size_t & max);

	void print(int indent = 0) const;
	void printMoveTuple(const IDType & player, const MoveTuple & m) const;
};

}

