#pragma once

#include "Common.h"

#include "AlphaBeta.h"
#include "GameState.h"
#include "Move.hpp"
#include "MoveArray.hpp"
#include "MicroSearchParameters.h"
#include "TranspositionTable.h"

#include <boost/shared_ptr.hpp>

namespace MicroSearch
{
typedef	boost::shared_ptr<TranspositionTable> TTPtr;

class Player 
{
protected:
	IDType _playerID;
public:
	virtual const MoveTuple getMoveTuple(GameState & state, const MoveArray & moves);
	virtual void			getMoves(GameState & state, const MoveArray & moves, std::vector<Move> & moveVec);
	const IDType ID();
	void setID(const IDType & playerid);
	virtual IDType getType() { return Search::PlayerModels::None; }
};

class AlphaBeta;
/*----------------------------------------------------------------------
 | Alpha Beta Player
 |----------------------------------------------------------------------
 | Runs Alpha Beta search given a set of search parameters
 `----------------------------------------------------------------------*/
class Player_AlphaBeta : public Player
{
	AlphaBeta * alphaBeta;
	TTPtr TT;
	MicroSearchParameters _params;
public:
	Player_AlphaBeta (const IDType & playerID);
	Player_AlphaBeta (const IDType & playerID, const MicroSearchParameters & params, TTPtr table);
	const MoveTuple getMoveTuple(GameState & state, const MoveArray & moves);
	void getMoves(GameState & state, const MoveArray & moves, std::vector<Move> & moveVec);
	void setParameters(MicroSearchParameters & p);
	const MicroSearchParameters & params() const;
	void setTranspositionTable(TTPtr table);
	const SearchResults & results() const;
	virtual ~Player_AlphaBeta();
	IDType getType() { return Search::PlayerModels::AlphaBeta; }
};

/*----------------------------------------------------------------------
 | Attack Closest Player
 |----------------------------------------------------------------------
 | Chooses an action with following priority:
 | 1) If it can attack, ATTACK closest enemy unit
 | 2) If it cannot attack:
 |    a) If it is in range to attack an enemy, WAIT until can shoot again
 |    b) If it is not in range of enemy, MOVE towards closest
 `----------------------------------------------------------------------*/
class Player_AttackClosest : public Player
{
public:
	Player_AttackClosest (const IDType & playerID);
	const MoveTuple getMoveTuple(GameState & state, const MoveArray & moves);
	void getMoves(GameState & state, const MoveArray & moves, std::vector<Move> & moveVec);
	IDType getType() { return Search::PlayerModels::AttackClosest; }
};

/*----------------------------------------------------------------------
 | Attack Weakest Player
 |----------------------------------------------------------------------
 | Chooses an action with following priority:
 | 1) If it can attack, ATTACK least hp enemy unit
 | 2) If it cannot attack:
 |    a) If it is in range to attack an enemy, WAIT
 |    b) If it is not in range of enemy, MOVE towards closest
 `----------------------------------------------------------------------*/
class Player_AttackWeakest : public Player
{
public:
	Player_AttackWeakest (const IDType & playerID);
	const MoveTuple getMoveTuple(GameState & state, const MoveArray & moves);
	void getMoves(GameState & state, const MoveArray & moves, std::vector<Move> & moveVec);
	IDType getType() { return Search::PlayerModels::AttackWeakest; }
};

/*----------------------------------------------------------------------
 | Kiter Player
 |----------------------------------------------------------------------
 | Chooses an action with following priority:
 | 1) If it can attack, ATTACK closest enemy unit
 | 2) If it cannot attack:
 |    a) If it is in range to attack an enemy, move away from closest
 |    b) If it is not in range of enemy, MOVE towards closest
 `----------------------------------------------------------------------*/
class Player_Kiter : public Player
{
public:
	Player_Kiter (const IDType & playerID);
	const MoveTuple getMoveTuple(GameState & state, const MoveArray & moves);
	void getMoves(GameState & state, const MoveArray & moves, std::vector<Move> & moveVec);
	IDType getType() { return Search::PlayerModels::Kiter; }
};

/*----------------------------------------------------------------------
 | Kiter DPS Player
 |----------------------------------------------------------------------
 | Chooses an action with following priority:
 | 1) If it can attack, ATTACK highest DPS/HP enemy unit in range
 | 2) If it cannot attack:
 |    a) If it is in range to attack an enemy, move away from closest one
 |    b) If it is not in range of enemy, MOVE towards closest one
 `----------------------------------------------------------------------*/
class Player_KiterDPS : public Player
{
public:
	Player_KiterDPS (const IDType & playerID);
	const MoveTuple getMoveTuple(GameState & state, const MoveArray & moves);
	void getMoves(GameState & state, const MoveArray & moves, std::vector<Move> & moveVec);
	IDType getType() { return Search::PlayerModels::KiterDPS; }
};

/*----------------------------------------------------------------------
 | Attack HighestDPS Player
 |----------------------------------------------------------------------
 | Chooses an action with following priority:
 | 1) If it can attack, ATTACK highest DPS/HP enemy unit in range
 | 2) If it cannot attack:
 |    a) If it is in range to attack an enemy, WAIT until attack
 |    b) If it is not in range of enemy, MOVE towards closest
 `----------------------------------------------------------------------*/
class Player_AttackDPS : public Player
{
public:
	Player_AttackDPS (const IDType & playerID);
	const MoveTuple getMoveTuple(GameState & state, const MoveArray & moves);
	void getMoves(GameState & state, const MoveArray & moves, std::vector<Move> & moveVec);
	IDType getType() { return Search::PlayerModels::AttackDPS; }
};

/*----------------------------------------------------------------------
 | Attack HighestDPS Player No Overkill
 |----------------------------------------------------------------------
 | Chooses an action with following priority:
 | 1) If it can attack, ATTACK highest DPS/HP enemy unit to overkill
 | 2) If it cannot attack:
 |    a) If it is in range to attack an enemy, WAIT until attack
 |    b) If it is not in range of enemy, MOVE towards closest
 `----------------------------------------------------------------------*/
class Player_NOK_AttackDPS : public Player
{
public:
	Player_NOK_AttackDPS (const IDType & playerID);
	const MoveTuple getMoveTuple(GameState & state, const MoveArray & moves);
	void getMoves(GameState & state, const MoveArray & moves, std::vector<Move> & moveVec);
	IDType getType() { return Search::PlayerModels::No_Overkill_DPS; }
};

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
/*----------------------------------------------------------------------
 | Random Player
 |----------------------------------------------------------------------
 | Chooses a random legal move per unit and implements it
 `----------------------------------------------------------------------*/
class Player_Random : public Player
{
	boost::random::mt19937 gen;
public:
	Player_Random (const IDType & playerID);
	const MoveTuple getMoveTuple(GameState & state, const MoveArray & moves);
	void getMoves(GameState & state, const MoveArray & moves, std::vector<Move> & moveVec);
	IDType getType() { return Search::PlayerModels::Random; }
};

namespace Players
{
	Player * getPlayer(const IDType & playerID, const IDType & playerType);
}

}


#include "Unit.hpp"
namespace MicroSearch
{
	class CompareUnitDPSThreat
	{
		const bool operator() (Unit * u1, Unit * u2) const
		{
			double u1Threat = ((double)u1->damage()/(double)u1->attackCooldown()) / u1->currentHP();
			double u2Threat = ((double)u2->damage()/(double)u2->attackCooldown()) / u2->currentHP();

			return u1Threat > u2Threat;
		}
	};
}