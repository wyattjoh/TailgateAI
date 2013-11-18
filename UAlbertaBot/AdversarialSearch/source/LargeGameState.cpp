#include "LargeGameState.h"
#include "Player.h"

#include "Game.h"

using namespace MicroSearch;

#define TABS(N) for (int i(0); i<N; ++i) { fprintf(stderr, "\t"); }

LargeGameState::LargeGameState()
	: _parent(NULL)
	, _currentTime(0)
	, _maxUnits(Search::Constants::Max_Units)
{
	_numUnits[0] = 0;
	_numUnits[1] = 0;
	_prevNumUnits[0] = 0;
	_prevNumUnits[1] = 0;
	_numMovements[0] = 0;
	_numMovements[1] = 0;

	for (size_t u(0); u<_maxUnits; ++u)
	{
		_unitPtrs[0][u] = &_units[0][u];
		_unitPtrs[1][u] = &_units[1][u];
	}
}

LargeGameState::LargeGameState(const LargeGameState & state)
	: _parent(state._parent)
	, _units(state._units)
	, _unitPtrs(state._unitPtrs)
	, _numUnits(state._numUnits)
	, _prevNumUnits(state._prevNumUnits)
	, _totalSumHP(state._totalSumHP)
	, _totalSumSQRT(state._totalSumSQRT)
	, _totalLTD2Bonus(state._totalLTD2Bonus)
	, _numMovements(state._numMovements)
	, _currentTime(state._currentTime)
	, _maxUnits(state._maxUnits)
{
	for (size_t u(0); u<_maxUnits; ++u)
	{
		int index0 = state.getIndexOfUnitPtr(0, u);
		int index1 = state.getIndexOfUnitPtr(1, u);
		_unitPtrs[0][u] = &_units[0][index0];
		_unitPtrs[1][u] = &_units[1][index1];
	}
}

// call this whenever we are done with moves
void LargeGameState::finishedMoving(bool normalize)
{
	// sort the unit vector based on time left to move
	sortUnits();

	//normalizeUnitPositions();

	// update the current time of the state
	updateGameTime();
}

void LargeGameState::normalizeUnitPositions()
{
	Unit * leftMostUnit = NULL;

	// for each player
	for (IDType p(0); p < Search::Constants::Num_Players; ++p)
	{
		// find the leftmost unit
		for (IDType u(0); u<_numUnits[p]; ++u)
		{
			if (!leftMostUnit || getUnit(p, u).x() < leftMostUnit->x())
			{
				leftMostUnit = &getUnit(p, u);
			}
		}
	}

	if (!leftMostUnit)
	{
		return;
	}

	PositionType leftMostX(leftMostUnit->x());

	// for each player
	for (IDType p(0); p < Search::Constants::Num_Players; ++p)
	{
		// for each unit they have, normalize them to x=0 starting with leftmost unit
		for (IDType u(0); u<_numUnits[p]; ++u)
		{
			getUnit(p, u).updatePosition(-leftMostX, 0);
		}
	}

	for (IDType p(0); p < Search::Constants::Num_Players; ++p)
	{
		// for each unit they have, normalize them to x=0 starting with leftmost unit
		for (IDType u(0); u<_numUnits[p]; ++u)
		{
			if (getUnit(p, u).x() < 0)
			{
				print();
			}
		}
	}
}

void LargeGameState::setMaxUnits(const size_t & size)
{
	
}

const size_t LargeGameState::getIndexOfUnitPtr(const size_t & player, const size_t & unit) const
{
	//const Unit * u_ptr_1 = _unitPtrs[player][unit];
	//const Unit * u_ptr_2 = &(_units[player][0]);
	return (_unitPtrs[player][unit] - &_units[player][0]);
}

const HashType LargeGameState::calculateHash(const size_t & hashNum) const
{
	HashType hash(0);

	for (IDType p(0); p < Search::Constants::Num_Players; ++p)
	{
		for (IDType u(0); u < _numUnits[p]; ++u)
		{
			hash ^= Hash::magicHash(getUnit(p,u).calculateHash(hashNum, _currentTime), p, u);
		}
	}

	return hash;
}
	
void LargeGameState::generateMoves(MoveArray & moves, const IDType & playerIndex) const
{
	moves.clear();

	// which is the enemy player
	IDType enemyPlayer  = getEnemy(playerIndex);

	// we are interested in all simultaneous moves
	// so return all units which can move at the same time as the first
	TimeType firstUnitMoveTime = getUnit(playerIndex, 0).firstTimeFree();
		
	for (IDType unitIndex(0); unitIndex < _numUnits[playerIndex]; ++unitIndex)
	{
		// unit reference
		const Unit & unit(getUnit(playerIndex,unitIndex));
			
		// if this unit can't move at the same time as the first
		if (unit.firstTimeFree() != firstUnitMoveTime)
		{
			// stop checking
			break;
		}

		moves.addUnit();

		// generate attack moves
		if (unit.canAttack())
		{
			for (IDType u(0); u<_numUnits[enemyPlayer]; ++u)
			{
				if (unit.canAttack(getUnit(enemyPlayer,u), _currentTime) && getUnit(enemyPlayer,u).isAlive())
				{
					moves.add(Move(unitIndex, playerIndex, MoveTypes::ATTACK, u));
				}
			}
		}
		// generate the wait move if it can't attack yet
		else
		{
			moves.add(Move(unitIndex, playerIndex, MoveTypes::WAIT, 0));
		}
		
		// generate movement moves
		for (IDType d(0); d<Search::Constants::Num_Directions; ++d)
		{
			Move move(unitIndex, playerIndex, MoveTypes::MOVE, d);
			
			Position dest;
			Position dir(Search::Constants::Move_Dir[move._moveIndex][0], 
						 Search::Constants::Move_Dir[move._moveIndex][1]);
			

			// if the unit can attack the destination will be set by default move distance
			if (unit.canAttack())
			{
				dest = Position(Search::Constants::Move_Distance*dir.x(), Search::Constants::Move_Distance*dir.y()) + unit.pos();
			}
			// otherwise it will be a move until attack move
			else
			{
				dest = unit.getMoveUntilAttackDest(dir, _currentTime);
			}

			if (!Search::Constants::Use_Unit_Bounding)
			{
				moves.add(Move(unitIndex, playerIndex, MoveTypes::MOVE, d));
			}
			else if (dest.x() > 0 && dest.x() < 1280 && dest.y() > 0 && dest.y() < 720)
			{
				moves.add(Move(unitIndex, playerIndex, MoveTypes::MOVE, d));
			}
		}
	}
}

void LargeGameState::makeMove(const Move & move, LargeGameState & parent)
{
	Unit & ourUnit		= getUnit(move._player, move._unit);
	IDType player		= ourUnit.player();
	IDType enemyPlayer  = getEnemy(player);

	if (move._moveType == MoveTypes::ATTACK)
	{
		Unit & enemyUnit(getUnit(enemyPlayer,move._moveIndex));
			
		// attack the unit
		ourUnit.attack(move, enemyUnit, _currentTime);
			
		// enemy unit takes damage if it is alive
		if (enemyUnit.isAlive())
		{				
			enemyUnit.takeAttack(ourUnit);

			// check to see if enemy unit died
			if (!enemyUnit.isAlive())
			{
				// if it died, remove it
				_numUnits[enemyPlayer]--;
			}
		}			
	}
	else if (move._moveType == MoveTypes::MOVE)
	{
		_numMovements[player]++;

		ourUnit.move(move, _currentTime);
	}
	else if (move._moveType == MoveTypes::WAIT)
	{
		ourUnit.waitUntilAttack(move, _currentTime);
	}

	setParent(&parent);
}

const IDType LargeGameState::getUnitIndex(const IDType & player, const Unit & unit) const
{
	for (IDType u(0); u<numUnits(player); ++u)
	{
		if (unit.ID() == getUnit(player, u).ID())
		{
			return u;
		}
	}

	return 255;
}

const Unit & LargeGameState::getUnitByID(const IDType & unitID) const
{
	for (IDType p(0); p<Search::Constants::Num_Players; ++p)
	{
		for (IDType u(0); u<numUnits(p); ++u)
		{
			if (getUnit(p, u).ID() == unitID)
			{
				return getUnit(p, u);
			}
		}
	}

	printf("UNIT DOES NOT EXIST YO\n");
	assert(false);
	return getUnit(0,0);
}

const Unit & LargeGameState::getUnitByID(const IDType & player, const IDType & unitID) const
{
	for (IDType u(0); u<numUnits(player); ++u)
	{
		if (getUnit(player, u).ID() == unitID)
		{
			return getUnit(player, u);
		}
	}

	printf("UNIT DOES NOT EXIST YO\n");
	assert(false);
	return getUnit(0,0);
}

const bool LargeGameState::checkBounds(const Position & pos) const
{
	return pos.x() || true;
}

const IDType LargeGameState::getEnemy(const IDType & player) const
{
	return (player + 1) % 2;
}

const Unit & LargeGameState::getClosestEnemyUnit(const IDType & player, const IDType & unitIndex)
{
	const IDType enemyPlayer(getEnemy(player));
	const Unit & myUnit(getUnit(player,unitIndex));

	size_t minDist(1000000);
	IDType minUnitInd(0);

	Position currentPos = myUnit.currentPosition(_currentTime);

	for (IDType u(0); u<_numUnits[enemyPlayer]; ++u)
	{
		//size_t distSq(myUnit.distSq(getUnit(enemyPlayer,u)));
		size_t distSq(currentPos.distSq(getUnit(enemyPlayer, u).currentPosition(_currentTime)));

		if (distSq < minDist)
		{
			minDist = distSq;
			minUnitInd = u;
		}
	}

	return getUnit(enemyPlayer, minUnitInd);
}
	
// add a unit to the game state, should only be called on construction
void LargeGameState::addUnit(const size_t & num, const BWAPI::UnitType unitType, const IDType & playerID, const Position & pos)
{
	for (size_t u(0); u<num; ++u)
	{
		getUnit(playerID,_numUnits[playerID]) = Unit(unitType, _numUnits[playerID], playerID, pos);
		_numUnits[playerID]++;
	}

	finishedMoving(false);
	calculateStartingHealth();
}

// add a unit to the game state, should only be called on construction
void LargeGameState::addUnitSymmetric(const size_t & num, const BWAPI::UnitType unitType, const IDType & playerID, const Position & pos)
{
	
	IDType enemy(getEnemy(playerID));
	Position oPos(pos);
	Position ePos(pos.flip());

	oPos = oPos + Position(512,384);
	ePos = ePos + Position(512,384);

	for (size_t u(0); u<num; ++u)
	{
		getUnit(playerID,_numUnits[playerID]) = Unit(unitType, _numUnits[playerID]+_numUnits[enemy], playerID, oPos);
		_numUnits[playerID]++;
	}
	
	for (size_t u(0); u<num; ++u)
	{
		getUnit(enemy,_numUnits[enemy]) = Unit(unitType, _numUnits[playerID]+_numUnits[enemy], enemy, ePos);
		_numUnits[enemy]++;
	}

	finishedMoving(false);
	calculateStartingHealth();
}

void LargeGameState::addUnit(const Unit & u)
{
	getUnit(u.player(), _numUnits[u.player()]) = u;
	_numUnits[u.player()]++;

	finishedMoving();
	calculateStartingHealth();
}

void LargeGameState::sortUnits()
{
	// sort the units based on time free
	for (size_t p(0); p<Search::Constants::Num_Players; ++p)
	{
		if (_prevNumUnits[p] <= 1)
		{
			_prevNumUnits[p] = _numUnits[p];
			continue;
		}
		else
		{
			std::sort(&_unitPtrs[p][0], &_unitPtrs[p][0] + _prevNumUnits[p], UnitPtrCompare());
			//_unitPtrs[p].sort(_prevNumUnits[p], UnitPtrCompare());
			_prevNumUnits[p] = _numUnits[p];
		}
	}	
}

Unit & LargeGameState::getUnit(const IDType & player, const UnitCountType & unit)
{
	return *_unitPtrs[player][unit];
}

const Unit & LargeGameState::getUnit(const IDType & player, const UnitCountType & unit) const
{
	return *_unitPtrs[player][unit];
}

const int LargeGameState::winner() const
{
	if (playerDead(Search::Players::Player_One))
	{
		return Search::Players::Player_Two;
	}
	else if (playerDead(Search::Players::Player_Two))
	{
		return Search::Players::Player_One;
	}
	else
	{
		return Search::Players::Player_None;
	}
}


const size_t LargeGameState::closestEnemyUnitDistance(const Unit & unit) const
{
	IDType enemyPlayer(getEnemy(unit.player()));

	size_t closestDist(0);

	for (IDType u(0); u<numUnits(enemyPlayer); ++u)
	{
		size_t dist(unit.distSq(getUnit(enemyPlayer, u), _currentTime));

		if (dist > closestDist)
		{
			closestDist = dist;
		}
	}

	return closestDist;
}

const bool LargeGameState::playerDead(const IDType & player) const
{
	return _numUnits[player] <= 0;
}

const IDType LargeGameState::whoCanMove() const
{
	// do something clever here (random()) if times are equal
	TimeType p1Time(getUnit(0,0).firstTimeFree());
	TimeType p2Time(getUnit(1,0).firstTimeFree());

	// if player one is to move first
	if (p1Time < p2Time)
	{
		return Search::Players::Player_One;
	}
	// if player two is to move first
	else if (p1Time > p2Time)
	{
		return Search::Players::Player_Two;
	}
	else
	{
		return Search::Players::Player_Both;
	}
}

void LargeGameState::updateGameTime()
{
	const IDType who(whoCanMove());

	// if the first player is to move, set the time to his time
	if (who == Search::Players::Player_One)
	{
		_currentTime = getUnit(Search::Players::Player_One, 0).firstTimeFree();
	}
	// otherwise it is player two or both, so it's equal to player two's time
	else
	{
		_currentTime = getUnit(Search::Players::Player_Two, 0).firstTimeFree();
	}
}

const AlphaBetaScore LargeGameState::eval(const IDType & player, const IDType & evalMethod, const IDType simMethod) const
{
	AlphaBetaScore score;
	const IDType enemyPlayer(getEnemy(player));

	// if both players are dead, return 0
	if (playerDead(enemyPlayer) && playerDead(player))
	{
		return AlphaBetaScore(0, 0);
	}

	AlphaBetaScore simEval;

	if (evalMethod == Search::EvaluationMethods::SumHP)
	{
		score = AlphaBetaScore(evalSumHP(player), 0);
	}
	else if (evalMethod == Search::EvaluationMethods::SumDPS)
	{
		score = AlphaBetaScore(evalSumDPS(player), 0);
	}
	else if (evalMethod == Search::EvaluationMethods::LTD2Bonus)
	{
		score = AlphaBetaScore(LTD2Bonus(player), 0);
	}
	else if (evalMethod == Search::EvaluationMethods::ModelSimulation)
	{
		score = evalSim(player, simMethod);
	}

	//printf("hpEval = %d\n", hpEval);

	if (score.val() == 0)
	{
		return score;
	}

	ScoreType winBonus(0);

	if (playerDead(enemyPlayer) && !playerDead(player))
	{
		winBonus = 100000;
	}
	else if (playerDead(player) && !playerDead(enemyPlayer))
	{
		winBonus = -100000;
	}

	return AlphaBetaScore(score.val() + winBonus, score.numMoves());
}

// evaluate the state for _playerToMove
const ScoreType LargeGameState::evalSumHP(const IDType & player) const
{
	const IDType enemyPlayer(getEnemy(player));
	
	return (ScoreType)(sumHP(player) - sumHP(enemyPlayer));
}

// evaluate the state for _playerToMove
const ScoreType LargeGameState::evalSumDPS(const IDType & player) const
{
	const IDType enemyPlayer(getEnemy(player));
	
	return (ScoreType)(sumDPS(player) - sumDPS(enemyPlayer));
}

// evaluate the state for _playerToMove
const ScoreType LargeGameState::evalLTD2Bonus(const IDType & player) const
{
	const IDType enemyPlayer(getEnemy(player));
	
	return (ScoreType)(LTD2Bonus(player) - LTD2Bonus(enemyPlayer));
}

const AlphaBetaScore LargeGameState::evalSim(const IDType & player, const IDType & method) const
{
	PlayerPtr p1(Players::getPlayer(Search::Players::Player_One, Search::PlayerModels::No_Overkill_DPS));
	PlayerPtr p2(Players::getPlayer(Search::Players::Player_Two, method));

	Game game(*this, p1, p2, 500);

	game.play();

	ScoreType evalReturn = game.getState().evalSumDPS(player);

	return AlphaBetaScore(evalReturn, game.getState().getNumMovements(player));
}

void LargeGameState::calculateStartingHealth()
{
	for (IDType p(0); p<Search::Constants::Num_Players; ++p)
	{
		float totalHP(0);
		float totalSQRT(0);
		float totalLTD2Bonus(0);

		for (IDType u(0); u<_numUnits[p]; ++u)
		{
			float dps = (float)getUnit(p,u).damage() / (float)getUnit(p,u).attackCooldown();

			totalHP += getUnit(p, u).maxHP() * dps;
			totalSQRT += sqrtf(getUnit(p,u).maxHP()) * dps;
			totalLTD2Bonus += sqrtf((float)(getUnit(p,u).maxHP() + (getUnit(p,u).maxHP()>>2))) * dps;
		}

		_totalSumHP[p] = totalHP;
		_totalSumSQRT[p] = totalSQRT;
		_totalLTD2Bonus[p] = totalSQRT;
	}
}

const ScoreType	LargeGameState::LTD2Bonus(const IDType & player) const
{
	if (numUnits(player) == 0)
	{
		return 0;
	}

	float sum(0);

	for (IDType u(0); u<_numUnits[player]; ++u)
	{
		const Unit & unit(getUnit(player, u));

		float dps = (float)unit.damage() / (float)unit.attackCooldown();

		sum += sqrtf((float)(unit.currentHP() + (unit.maxHP()>>2))) * dps;
	}

	return (ScoreType)(1000 * sum / _totalLTD2Bonus[player]);
}

const ScoreType	LargeGameState::sumDPS(const IDType & player) const
{
	if (numUnits(player) == 0)
	{
		return 0;
	}

	float sum(0);

	for (IDType u(0); u<_numUnits[player]; ++u)
	{
		float dps = (float)getUnit(player,u).damage() / (float)getUnit(player,u).attackCooldown();

		sum += sqrtf(getUnit(player,u).currentHP()) * dps;
	}

	return (ScoreType)(1000 * sum / _totalSumSQRT[player]);
}

const ScoreType LargeGameState::sumHP(const IDType & player) const
{
	if (numUnits(player) == 0)
	{
		return 0;
	}

	float sum(0);

	for (IDType u(0); u<_numUnits[player]; ++u)
	{
		float dps = (float)getUnit(player,u).damage() / (float)getUnit(player,u).attackCooldown();

		sum += getUnit(player, u).currentHP() * dps;
	}

	return (ScoreType)(1000 * sum / _totalSumHP[player]);
}

const size_t LargeGameState::numUnits(const IDType & player) const
{
	return _numUnits[player];
}

const Unit & LargeGameState::getUnitDirect(const IDType & player, const IDType & unit) const
{
	return _units[player][unit];
}

const bool LargeGameState::bothCanMove() const
{
	return getUnit(0, 0).firstTimeFree() == getUnit(1, 0).firstTimeFree();
}

void LargeGameState::setTime(const TimeType & time)
{
	_currentTime = time;
}

const int & LargeGameState::getNumMovements(const IDType & player) const
{
	return _numMovements[player];
}

void LargeGameState::setParent(LargeGameState * parent)
{
	_parent = parent;
}

LargeGameState * LargeGameState::getParent() const
{
	return _parent;
}

const TimeType LargeGameState::getTime() const
{
	return _currentTime;
}

const float & LargeGameState::getTotalSumHP(const IDType & player) const
{
	return _totalSumHP[player];
}

const float & LargeGameState::getTotalSumDPS(const IDType & player)	const
{
	return _totalSumSQRT[player];
}

void LargeGameState::setTotalSumHP(const float & p1, const float & p2)
{
	_totalSumHP[Search::Players::Player_One] = p1;
	_totalSumHP[Search::Players::Player_Two] = p2;
}

void LargeGameState::setTotalSumDPS(const float & p1, const float & p2)
{
	_totalSumSQRT[Search::Players::Player_One] = p1;
	_totalSumSQRT[Search::Players::Player_Two] = p2;
}

bool LargeGameState::matches(const LargeGameState & s) const
{
	for (IDType p(0); p<2; ++p)
	{
		for (IDType u(0); u<_numUnits[p]; ++u)
		{
			const Unit & u1(getUnit(p,u));
			const Unit & u2(s.getUnit(p,u));

			if (u1.x() != u2.x())
			{
				std::cout << "--------------HOLY SHIT \n";
				u1.print();
				u2.print();
				u1.debugHash(0, _currentTime); u1.debugHash(1, _currentTime);
				u2.debugHash(0, _currentTime); u2.debugHash(1, _currentTime);
				print();
				s.print();

				return true;
			}
		}
	}
	
	return true;
}

// print the state in a neat way
void LargeGameState::print(int indent) const
{
	
	TABS(indent);
	std::cout << calculateHash(0) << "\n";
	fprintf(stderr, "State - Time: %d\n", _currentTime);

	for (IDType p(0); p<Search::Constants::Num_Players; ++p)
	{
		for (UnitCountType u(0); u<_numUnits[p]; ++u)
		{
			const Unit & unit(getUnit(p, u));

			TABS(indent);
			fprintf(stderr, "  P%d %5d %5d    (%3d, %3d)     %s\n", unit.player(), unit.currentHP(), unit.firstTimeFree(), unit.x(), unit.y(), unit.name().c_str());
		}
	}
	fprintf(stderr, "\n\n");
}

void LargeGameState::printMoveTuple(const IDType & player, const MoveTuple & t) const
{
	printf("\n");
	MoveArray moves;
	generateMoves(moves, player);
	for (size_t u(0); u<moves.numUnitsInTuple(); ++u)
	{
		Move m = moves.getTupleMove(t, u);
		
		std::cout << "Player " << (int)m.player() << " " << getUnit(m.player(), m.unit()).name() << " (id=" << (int)m.unit() << ")" << " " << m.moveString() ;
		if (m.type() == MoveTypes::ATTACK)
		{
			std::cout << " target " << getUnit(getEnemy(m.player()), m.index()).name() << " (id=" << (int)m.index() << ")";
		}

		std::cout << "\n";
	}
	
}