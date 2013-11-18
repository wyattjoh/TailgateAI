#include "Player.h"

using namespace MicroSearch;


Player * MicroSearch::Players::getPlayer(const IDType & playerID, const IDType & type)
{
	if			(type == Search::PlayerModels::AttackClosest)		{ return new Player_AttackClosest(playerID); }
	else if		(type == Search::PlayerModels::AttackDPS)			{ return new Player_AttackDPS(playerID); }
	else if		(type == Search::PlayerModels::AttackWeakest)		{ return new Player_AttackWeakest(playerID); }
	else if		(type == Search::PlayerModels::Kiter)				{ return new Player_Kiter(playerID); }
	else if		(type == Search::PlayerModels::KiterDPS)			{ return new Player_KiterDPS(playerID); }
	else if		(type == Search::PlayerModels::No_Overkill_DPS)		{ return new Player_NOK_AttackDPS(playerID); }
	else															{ return NULL; }
}


const MoveTuple Player::getMoveTuple(GameState & state, const MoveArray & moves) 
{ 
	return 0; 
}

void Player::getMoves(GameState & state, const MoveArray & moves, std::vector<Move> & moveVec)
{
	// not implemented
}

const IDType Player::ID() 
{ 
	return _playerID; 
}

void Player::setID(const IDType & playerID)
{
	_playerID = playerID;
}

////////////////////////////////////////
// Alpha-Beta Player
////////////////////////////////////////
Player_AlphaBeta::Player_AlphaBeta (const IDType & playerID) 
{
	_playerID = playerID;
}

Player_AlphaBeta::Player_AlphaBeta (const IDType & playerID, const MicroSearchParameters & params, TTPtr table)
{
	_playerID = playerID;
	_params = params;
	TT = table;

	alphaBeta = new AlphaBeta(_params, TT);
}

const SearchResults & Player_AlphaBeta::results() const
{
	return alphaBeta->getResults();
}

const MicroSearchParameters & Player_AlphaBeta::params() const
{
	return _params;
}

Player_AlphaBeta::~Player_AlphaBeta()
{
	delete alphaBeta;
}

void Player_AlphaBeta::setParameters(MicroSearchParameters & p)
{
	_params = p;
}

void Player_AlphaBeta::setTranspositionTable(TTPtr table)
{
	TT = table;
}

void Player_AlphaBeta::getMoves(GameState & state, const MoveArray & moves, std::vector<Move> & moveVec)
{
	// not implemented
}

const MoveTuple Player_AlphaBeta::getMoveTuple(GameState & state, const MoveArray & moves)
{
	alphaBeta->doSearch(state);

	return alphaBeta->getResults().bestMoveTuple;
}

Player_AttackClosest::Player_AttackClosest (const IDType & playerID) 
{
	_playerID = playerID;
}

const MoveTuple Player_AttackClosest::getMoveTuple(GameState & state, const MoveArray & moves)
{
	MoveTuple tuple(0);

	for (IDType u(0); u<moves.numUnits(); ++u)
	{
		bool foundAction					(false);
		size_t actionMoveIndex				(0);
		size_t closestMoveIndex				(0);
		unsigned long long actionDistance	(std::numeric_limits<unsigned long long>::max());
		unsigned long long closestMoveDist	(std::numeric_limits<unsigned long long>::max());

		const Unit & ourUnit				(state.getUnit(_playerID, u));
		const Unit & closestUnit			(ourUnit.canHeal() ? state.getClosestOurUnit(_playerID, u) : state.getClosestEnemyUnit(_playerID, u));

		for (size_t m(0); m<moves.numMoves(u); ++m)
		{
			const Move move					(moves.getMove(u, m));
				
			if (move.type() == MoveTypes::ATTACK)
			{
				const Unit & target			(state.getUnit(state.getEnemy(move.player()), move._moveIndex));
				size_t dist					(ourUnit.distSq(target, state.getTime()));

				if (dist < actionDistance)
				{
					actionDistance = dist;
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			if (move.type() == MoveTypes::HEAL)
			{
				const Unit & target			(state.getUnit(move.player(), move._moveIndex));
				size_t dist					(ourUnit.distSq(target, state.getTime()));

				if (dist < actionDistance)
				{
					actionDistance = dist;
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			else if (move.type() == MoveTypes::RELOAD)
			{
				if (ourUnit.canAttackTarget(closestUnit, state.getTime()))
				{
					closestMoveIndex = m;
					break;
				}
			}
			else if (move.type() == MoveTypes::MOVE)
			{
				Position ourDest			(ourUnit.x() + Search::Constants::Move_Dir[move._moveIndex][0], 
											 ourUnit.y() + Search::Constants::Move_Dir[move._moveIndex][1]);
				size_t dist					(closestUnit.distSq(ourDest, state.getTime()));

				if (dist < closestMoveDist)
				{
					closestMoveDist = dist;
					closestMoveIndex = m;
				}
			}
		}

		size_t bestMoveIndex(foundAction ? actionMoveIndex : closestMoveIndex);

		tuple += bestMoveIndex * moves.getProduct(u);
	}

	return tuple;
}

void Player_AttackClosest::getMoves(GameState & state, const MoveArray & moves, std::vector<Move> & moveVec)
{
	for (IDType u(0); u<moves.numUnits(); ++u)
	{
		bool foundAction					(false);
		size_t actionMoveIndex				(0);
		size_t closestMoveIndex				(0);
		unsigned long long actionDistance	(std::numeric_limits<unsigned long long>::max());
		unsigned long long closestMoveDist	(std::numeric_limits<unsigned long long>::max());

		const Unit & ourUnit				(state.getUnit(_playerID, u));
		const Unit & closestUnit			(ourUnit.canHeal() ? state.getClosestOurUnit(_playerID, u) : state.getClosestEnemyUnit(_playerID, u));

		for (size_t m(0); m<moves.numMoves(u); ++m)
		{
			const Move move					(moves.getMove(u, m));
				
			if (move.type() == MoveTypes::ATTACK)
			{
				const Unit & target			(state.getUnit(state.getEnemy(move.player()), move._moveIndex));
				size_t dist					(ourUnit.distSq(target, state.getTime()));

				if (dist < actionDistance)
				{
					actionDistance = dist;
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			if (move.type() == MoveTypes::HEAL)
			{
				const Unit & target			(state.getUnit(move.player(), move._moveIndex));
				size_t dist					(ourUnit.distSq(target, state.getTime()));

				if (dist < actionDistance)
				{
					actionDistance = dist;
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			else if (move.type() == MoveTypes::RELOAD)
			{
				if (ourUnit.canAttackTarget(closestUnit, state.getTime()))
				{
					closestMoveIndex = m;
					break;
				}
			}
			else if (move.type() == MoveTypes::MOVE)
			{
				Position ourDest			(ourUnit.x() + Search::Constants::Move_Dir[move._moveIndex][0], 
											 ourUnit.y() + Search::Constants::Move_Dir[move._moveIndex][1]);
				size_t dist					(closestUnit.distSq(ourDest, state.getTime()));

				if (dist < closestMoveDist)
				{
					closestMoveDist = dist;
					closestMoveIndex = m;
				}
			}
		}

		size_t bestMoveIndex(foundAction ? actionMoveIndex : closestMoveIndex);

		moveVec.push_back(moves.getMove(u, bestMoveIndex));
	}
}

Player_AttackWeakest::Player_AttackWeakest (const IDType & playerID) 
{
	_playerID = playerID;
}

const MoveTuple Player_AttackWeakest::getMoveTuple(GameState & state, const MoveArray & moves)
{
	MoveTuple tuple(0);

	for (IDType u(0); u<moves.numUnits(); ++u)
	{
		bool foundAction						(false);
		size_t actionMoveIndex					(0);
		size_t actionLowestHP					(1000000);
		size_t closestMoveIndex					(0);
		unsigned long long closestMoveDist		(std::numeric_limits<unsigned long long>::max());
		
		const Unit & ourUnit				(state.getUnit(_playerID, u));
		const Unit & closestUnit			(ourUnit.canHeal() ? state.getClosestOurUnit(_playerID, u) : state.getClosestEnemyUnit(_playerID, u));

		for (size_t m(0); m<moves.numMoves(u); ++m)
		{
			const Move move						(moves.getMove(u, m));
				
			if (move.type() == MoveTypes::ATTACK)
			{
				const Unit & target				(state.getUnit(state.getEnemy(move.player()), move._moveIndex));

				if ((size_t)target.currentHP() < actionLowestHP)
				{
					actionLowestHP = target.currentHP();
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			else if (move.type() == MoveTypes::HEAL)
			{
				const Unit & target				(state.getUnit(move.player(), move._moveIndex));

				if ((size_t)target.currentHP() < actionLowestHP)
				{
					actionLowestHP = target.currentHP();
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			else if (move.type() == MoveTypes::RELOAD)
			{
				if (ourUnit.canAttackTarget(closestUnit, state.getTime()))
				{
					closestMoveIndex = m;
					break;
				}
			}
			else if (move.type() == MoveTypes::MOVE)
			{
				Position ourDest				(ourUnit.x() + Search::Constants::Move_Dir[move._moveIndex][0], 
												 ourUnit.y() + Search::Constants::Move_Dir[move._moveIndex][1]);
				size_t dist						(closestUnit.distSq(ourDest, state.getTime()));

				if (dist < closestMoveDist)
				{
					closestMoveDist = dist;
					closestMoveIndex = m;
				}
			}
		}

		size_t bestMoveIndex(foundAction ? actionMoveIndex : closestMoveIndex);

		tuple += bestMoveIndex * moves.getProduct(u);
	}

	return tuple;
}

void Player_AttackWeakest::getMoves(GameState & state, const MoveArray & moves, std::vector<Move> & moveVec)
{
	for (IDType u(0); u<moves.numUnits(); ++u)
	{
		bool foundAction						(false);
		size_t actionMoveIndex					(0);
		size_t closestMoveIndex					(0);
		size_t actionLowestHP					(1000000);
		unsigned long long closestMoveDist		(std::numeric_limits<unsigned long long>::max());
		
		const Unit & ourUnit				(state.getUnit(_playerID, u));
		const Unit & closestUnit			(ourUnit.canHeal() ? state.getClosestOurUnit(_playerID, u) : state.getClosestEnemyUnit(_playerID, u));

		for (size_t m(0); m<moves.numMoves(u); ++m)
		{
			const Move move						(moves.getMove(u, m));
				
			if (move.type() == MoveTypes::ATTACK)
			{
				const Unit & target				(state.getUnit(state.getEnemy(move.player()), move._moveIndex));

				if ((size_t)target.currentHP() < actionLowestHP)
				{
					actionLowestHP = target.currentHP();
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			else if (move.type() == MoveTypes::HEAL)
			{
				const Unit & target				(state.getUnit(move.player(), move._moveIndex));

				if ((size_t)target.currentHP() < actionLowestHP)
				{
					actionLowestHP = target.currentHP();
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			else if (move.type() == MoveTypes::RELOAD)
			{
				if (ourUnit.canAttackTarget(closestUnit, state.getTime()))
				{
					closestMoveIndex = m;
					break;
				}
			}
			else if (move.type() == MoveTypes::MOVE)
			{
				Position ourDest				(ourUnit.x() + Search::Constants::Move_Dir[move._moveIndex][0], 
												 ourUnit.y() + Search::Constants::Move_Dir[move._moveIndex][1]);
				size_t dist						(closestUnit.distSq(ourDest, state.getTime()));

				if (dist < closestMoveDist)
				{
					closestMoveDist = dist;
					closestMoveIndex = m;
				}
			}
		}

		size_t bestMoveIndex(foundAction ? actionMoveIndex : closestMoveIndex);
			
		moveVec.push_back(moves.getMove(u, bestMoveIndex));
	}
}

Player_AttackDPS::Player_AttackDPS (const IDType & playerID) 
{
	_playerID = playerID;
}

void Player_AttackDPS::getMoves(GameState & state, const MoveArray & moves, std::vector<Move> & moveVec)
{
	for (IDType u(0); u<moves.numUnits(); ++u)
	{
		bool foundAction						(false);
		size_t actionMoveIndex					(0);
		size_t closestMoveIndex					(0);
		double actionHighestDPS					(0);
		unsigned long long closestMoveDist		(std::numeric_limits<unsigned long long>::max());
		
		const Unit & ourUnit					(state.getUnit(_playerID, u));
		const Unit & closestUnit				(ourUnit.canHeal() ? state.getClosestOurUnit(_playerID, u) : state.getClosestEnemyUnit(_playerID, u));

		for (size_t m(0); m<moves.numMoves(u); ++m)
		{
			const Move move						(moves.getMove(u, m));
				
			if (move.type() == MoveTypes::ATTACK)
			{
				const Unit & target				(state.getUnit(state.getEnemy(move.player()), move._moveIndex));
				double dpsHPValue =				(target.dpf() / target.currentHP());

				if (dpsHPValue > actionHighestDPS)
				{
					actionHighestDPS = dpsHPValue;
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			if (move.type() == MoveTypes::HEAL)
			{
				const Unit & target				(state.getUnit(move.player(), move._moveIndex));
				double dpsHPValue =				(target.dpf() / target.currentHP());

				if (dpsHPValue > actionHighestDPS)
				{
					actionHighestDPS = dpsHPValue;
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			else if (move.type() == MoveTypes::RELOAD)
			{
				if (ourUnit.canAttackTarget(closestUnit, state.getTime()))
				{
					closestMoveIndex = m;
					break;
				}
			}
			else if (move.type() == MoveTypes::MOVE)
			{
				Position ourDest				(ourUnit.x() + Search::Constants::Move_Dir[move._moveIndex][0], 
												 ourUnit.y() + Search::Constants::Move_Dir[move._moveIndex][1]);
				size_t dist						(closestUnit.distSq(ourDest, state.getTime()));

				if (dist < closestMoveDist)
				{
					closestMoveDist = dist;
					closestMoveIndex = m;
				}
			}
		}

		size_t bestMoveIndex(foundAction ? actionMoveIndex : closestMoveIndex);
			
		moveVec.push_back(moves.getMove(u, bestMoveIndex));
	}
}

const MoveTuple Player_AttackDPS::getMoveTuple(GameState & state, const MoveArray & moves)
{
	MoveTuple tuple(0);

	for (IDType u(0); u<moves.numUnits(); ++u)
	{
		bool foundAction						(false);
		size_t actionMoveIndex					(0);
		size_t closestMoveIndex					(0);
		double actionHighestDPS					(0);
		unsigned long long closestMoveDist		(std::numeric_limits<unsigned long long>::max());
		
		const Unit & ourUnit				(state.getUnit(_playerID, u));
		const Unit & closestUnit			(ourUnit.canHeal() ? state.getClosestOurUnit(_playerID, u) : state.getClosestEnemyUnit(_playerID, u));

		for (size_t m(0); m<moves.numMoves(u); ++m)
		{
			const Move move						(moves.getMove(u, m));
				
			if (move.type() == MoveTypes::ATTACK)
			{
				const Unit & target				(state.getUnit(state.getEnemy(move.player()), move._moveIndex));
				double dpsHPValue =				(target.dpf() / target.currentHP());

				if (dpsHPValue > actionHighestDPS)
				{
					actionHighestDPS = dpsHPValue;
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			if (move.type() == MoveTypes::HEAL)
			{
				const Unit & target				(state.getUnit(move.player(), move._moveIndex));
				double dpsHPValue =				(target.dpf() / target.currentHP());

				if (dpsHPValue > actionHighestDPS)
				{
					actionHighestDPS = dpsHPValue;
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			else if (move.type() == MoveTypes::RELOAD)
			{
				if (ourUnit.canAttackTarget(closestUnit, state.getTime()))
				{
					closestMoveIndex = m;
					break;
				}
			}
			else if (move.type() == MoveTypes::MOVE)
			{
				Position ourDest				(ourUnit.x() + Search::Constants::Move_Dir[move._moveIndex][0], 
												 ourUnit.y() + Search::Constants::Move_Dir[move._moveIndex][1]);
				size_t dist						(closestUnit.distSq(ourDest, state.getTime()));

				if (dist < closestMoveDist)
				{
					closestMoveDist = dist;
					closestMoveIndex = m;
				}
			}
		}

		size_t bestMoveIndex(foundAction ? actionMoveIndex : closestMoveIndex);
			
		tuple += bestMoveIndex * moves.getProduct(u);
	}

	return tuple;
}

Player_NOK_AttackDPS::Player_NOK_AttackDPS (const IDType & playerID) 
{
	_playerID = playerID;
}

void Player_NOK_AttackDPS::getMoves(GameState & state, const MoveArray & moves, std::vector<Move> & moveVec)
{
	IDType player(moves.getMove(0,0).player());
	IDType enemy(state.getEnemy(player));

	Array<int, Search::Constants::Max_Units> hpRemaining;
	if (state.numUnits(enemy) > Search::Constants::Max_Units) 
	{
		hpRemaining.resize(state.numUnits(enemy));
	}

	for (IDType u(0); u<state.numUnits(enemy); ++u)
	{
		hpRemaining[u] = state.getUnit(enemy,u).currentHP();
	}

	for (IDType u(0); u<moves.numUnits(); ++u)
	{
		bool foundAction						(false);
		size_t actionMoveIndex					(0);
		double actionHighestDPS					(0);
		size_t closestMoveIndex					(0);
		unsigned long long closestMoveDist		(std::numeric_limits<unsigned long long>::max());
		
		const Unit & ourUnit				(state.getUnit(_playerID, u));
		const Unit & closestUnit			(ourUnit.canHeal() ? state.getClosestOurUnit(_playerID, u) : state.getClosestEnemyUnit(_playerID, u));

		for (size_t m(0); m<moves.numMoves(u); ++m)
		{
			const Move move						(moves.getMove(u, m));
				
			if ((move.type() == MoveTypes::ATTACK) && (hpRemaining[move._moveIndex] > 0))
			{
				const Unit & target				(state.getUnit(state.getEnemy(move.player()), move._moveIndex));
				double dpsHPValue =				(target.dpf() / hpRemaining[move._moveIndex]);

				if (dpsHPValue > actionHighestDPS)
				{
					actionHighestDPS = dpsHPValue;
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			else if (move.type() == MoveTypes::HEAL)
			{
				const Unit & target				(state.getUnit(move.player(), move._moveIndex));
				double dpsHPValue =				(target.dpf() / hpRemaining[move._moveIndex]);

				if (dpsHPValue > actionHighestDPS)
				{
					actionHighestDPS = dpsHPValue;
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			else if (move.type() == MoveTypes::RELOAD)
			{
				if (ourUnit.canAttackTarget(closestUnit, state.getTime()))
				{
					closestMoveIndex = m;
					break;
				}
			}
			else if (move.type() == MoveTypes::MOVE)
			{
				Position ourDest				(ourUnit.x() + Search::Constants::Move_Dir[move._moveIndex][0], 
												 ourUnit.y() + Search::Constants::Move_Dir[move._moveIndex][1]);
				size_t dist						(closestUnit.distSq(ourDest, state.getTime()));

				if (dist < closestMoveDist)
				{
					closestMoveDist = dist;
					closestMoveIndex = m;
				}
			}
		}

		size_t bestMoveIndex(foundAction ? actionMoveIndex : closestMoveIndex);

		Move theMove(moves.getMove(u, actionMoveIndex));
		if (theMove.type() == MoveTypes::ATTACK)
		{
			hpRemaining[theMove.index()] -= state.getUnit(player, theMove.unit()).damage();
		}
			
		moveVec.push_back(moves.getMove(u, bestMoveIndex));
	}
}

const MoveTuple Player_NOK_AttackDPS::getMoveTuple(GameState & state, const MoveArray & moves)
{
	MoveTuple tuple(0);

	IDType player(moves.getMove(0,0).player());
	IDType enemy(state.getEnemy(player));

	Array<int, Search::Constants::Max_Units> hpRemaining;
	if (state.numUnits(enemy) > Search::Constants::Max_Units) 
	{
		hpRemaining.resize(state.numUnits(enemy));
	}

	for (IDType u(0); u<state.numUnits(enemy); ++u)
	{
		hpRemaining[u] = state.getUnit(enemy,u).currentHP();
	}

	for (IDType u(0); u<moves.numUnits(); ++u)
	{
		bool foundAction						(false);
		size_t actionMoveIndex					(0);
		double actionHighestDPS					(0);
		size_t closestMoveIndex					(0);
		unsigned long long closestMoveDist		(std::numeric_limits<unsigned long long>::max());
		
		const Unit & ourUnit				(state.getUnit(_playerID, u));
		const Unit & closestUnit			(ourUnit.canHeal() ? state.getClosestOurUnit(_playerID, u) : state.getClosestEnemyUnit(_playerID, u));

		for (size_t m(0); m<moves.numMoves(u); ++m)
		{
			const Move move						(moves.getMove(u, m));
				
			if ((move.type() == MoveTypes::ATTACK) && (hpRemaining[move._moveIndex] > 0))
			{
				const Unit & target				(state.getUnit(state.getEnemy(move.player()), move._moveIndex));
				double dpsHPValue =				(target.dpf() / hpRemaining[move._moveIndex]);

				if (dpsHPValue > actionHighestDPS)
				{
					actionHighestDPS = dpsHPValue;
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			else if (move.type() == MoveTypes::HEAL)
			{
				const Unit & target				(state.getUnit(move.player(), move._moveIndex));
				double dpsHPValue =				(target.dpf() / hpRemaining[move._moveIndex]);

				if (dpsHPValue > actionHighestDPS)
				{
					actionHighestDPS = dpsHPValue;
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			else if (move.type() == MoveTypes::RELOAD)
			{
				if (ourUnit.canAttackTarget(closestUnit, state.getTime()))
				{
					closestMoveIndex = m;
					break;
				}
			}
			else if (move.type() == MoveTypes::MOVE)
			{
				Position ourDest				(ourUnit.x() + Search::Constants::Move_Dir[move._moveIndex][0], 
												 ourUnit.y() + Search::Constants::Move_Dir[move._moveIndex][1]);
				size_t dist						(closestUnit.distSq(ourDest, state.getTime()));

				if (dist < closestMoveDist)
				{
					closestMoveDist = dist;
					closestMoveIndex = m;
				}
			}
		}

		size_t bestMoveIndex(foundAction ? actionMoveIndex : closestMoveIndex);

		Move theMove(moves.getMove(u, actionMoveIndex));
		if (theMove.type() == MoveTypes::ATTACK)
		{
			hpRemaining[theMove.index()] -= state.getUnit(player, theMove.unit()).damage();
		}
			
		tuple += bestMoveIndex * moves.getProduct(u);
	}

	return tuple;
}

////////////////////////////////////////
// Kiter Player
////////////////////////////////////////
Player_Kiter::Player_Kiter (const IDType & playerID) 
{
	_playerID = playerID;
}

void Player_Kiter::getMoves(GameState & state, const MoveArray & moves, std::vector<Move> & moveVec)
{
	for (IDType u(0); u<moves.numUnits(); ++u)
	{
		bool foundAction						(false);
		IDType actionMoveIndex					(0);
		IDType furthestMoveIndex				(0);
		size_t furthestMoveDist					(0);
		IDType closestMoveIndex					(0);
		int actionDistance						(std::numeric_limits<int>::max());
		unsigned long long closestMoveDist		(std::numeric_limits<unsigned long long>::max());

		const Unit & ourUnit					(state.getUnit(_playerID, u));
		const Unit & closestUnit				(ourUnit.canHeal() ? state.getClosestOurUnit(_playerID, u) : state.getClosestEnemyUnit(_playerID, u));

		for (IDType m(0); m<moves.numMoves(u); ++m)
		{
			const Move move						(moves.getMove(u, m));
				
			if (move.type() == MoveTypes::ATTACK)
			{
				const Unit & target				(state.getUnit(state.getEnemy(move.player()), move._moveIndex));
				PositionType dist				(ourUnit.distSq(target, state.getTime()));

				if (dist < actionDistance)
				{
					actionDistance = dist;
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			else if (move.type() == MoveTypes::HEAL)
			{
				const Unit & target				(state.getUnit(move.player(), move._moveIndex));
				PositionType dist				(ourUnit.distSq(target, state.getTime()));

				if (dist < actionDistance)
				{
					actionDistance = dist;
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			else if (move.type() == MoveTypes::MOVE)
			{
				Position ourDest				(ourUnit.x() + Search::Constants::Move_Dir[move._moveIndex][0], 
												 ourUnit.y() + Search::Constants::Move_Dir[move._moveIndex][1]);
				size_t dist						(closestUnit.distSq(ourDest, state.getTime()));

				if (dist > furthestMoveDist)
				{
					furthestMoveDist = dist;
					furthestMoveIndex = m;
				}

				if (dist < closestMoveDist)
				{
					closestMoveDist = dist;
					closestMoveIndex = m;
				}
			}
		}

		// the move we will be returning
		size_t bestMoveIndex(0);

		// if we have an attack move we will use that one
		if (foundAction)
		{
			bestMoveIndex = actionMoveIndex;
		}
		// otherwise use the closest move to the opponent
		else
		{
			// if we are in attack range of the unit, back up
			if (ourUnit.canAttackTarget(closestUnit, state.getTime()))
			{
				bestMoveIndex = furthestMoveIndex;
			}
			// otherwise get back into the fight
			else
			{
				bestMoveIndex = closestMoveIndex;
			}
		}
			
		moveVec.push_back(moves.getMove(u, bestMoveIndex));
	}
}

// TODO: UNTESTED
const MoveTuple Player_Kiter::getMoveTuple(GameState & state, const MoveArray & moves)
{
	// the tuple, we will generate this on the fly
	MoveTuple tuple(0);
	
	for (IDType u(0); u<moves.numUnits(); ++u)
	{
		bool foundAction						(false);
		IDType actionMoveIndex					(0);
		IDType furthestMoveIndex				(0);
		size_t furthestMoveDist					(0);
		IDType closestMoveIndex					(0);
		int actionDistance						(std::numeric_limits<int>::max());
		unsigned long long closestMoveDist		(std::numeric_limits<unsigned long long>::max());

		const Unit & ourUnit					(state.getUnit(_playerID, u));
		const Unit & closestUnit				(ourUnit.canHeal() ? state.getClosestOurUnit(_playerID, u) : state.getClosestEnemyUnit(_playerID, u));

		for (IDType m(0); m<moves.numMoves(u); ++m)
		{
			const Move move						(moves.getMove(u, m));
				
			if (move.type() == MoveTypes::ATTACK)
			{
				const Unit & target				(state.getUnit(state.getEnemy(move.player()), move._moveIndex));
				PositionType dist				(ourUnit.distSq(target, state.getTime()));

				if (dist < actionDistance)
				{
					actionDistance = dist;
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			else if (move.type() == MoveTypes::HEAL)
			{
				const Unit & target				(state.getUnit(move.player(), move._moveIndex));
				PositionType dist				(ourUnit.distSq(target, state.getTime()));

				if (dist < actionDistance)
				{
					actionDistance = dist;
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			else if (move.type() == MoveTypes::MOVE)
			{
				Position ourDest				(ourUnit.x() + Search::Constants::Move_Dir[move._moveIndex][0], 
												 ourUnit.y() + Search::Constants::Move_Dir[move._moveIndex][1]);
				size_t dist						(closestUnit.distSq(ourDest, state.getTime()));

				if (dist > furthestMoveDist)
				{
					furthestMoveDist = dist;
					furthestMoveIndex = m;
				}

				if (dist < closestMoveDist)
				{
					closestMoveDist = dist;
					closestMoveIndex = m;
				}
			}
		}

		// the move we will be returning
		size_t bestMoveIndex(0);

		// if we have an attack move we will use that one
		if (foundAction)
		{
			bestMoveIndex = actionMoveIndex;
		}
		// otherwise use the closest move to the opponent
		else
		{
			// if we are in attack range of the unit, back up
			if (ourUnit.canAttackTarget(closestUnit, state.getTime()))
			{
				bestMoveIndex = furthestMoveIndex;
			}
			// otherwise get back into the fight
			else
			{
				bestMoveIndex = closestMoveIndex;
			}
		}
			
		// update the tuple calculation
		tuple += bestMoveIndex * moves.getProduct(u);
	}

	return tuple;
}

////////////////////////////////////////
// Kiter DPS
////////////////////////////////////////
Player_KiterDPS::Player_KiterDPS (const IDType & playerID) 
{
	_playerID = playerID;
}

void Player_KiterDPS::getMoves(GameState & state, const MoveArray & moves, std::vector<Move> & moveVec)
{
	for (IDType u(0); u<moves.numUnits(); ++u)
	{
		bool foundAction						(false);
		IDType actionMoveIndex					(0);
		IDType furthestMoveIndex				(0);
		size_t furthestMoveDist					(0);
		IDType closestMoveIndex					(0);
		double actionHighestDPS					(0);
		unsigned long long closestMoveDist		(std::numeric_limits<unsigned long long>::max());

		const Unit & ourUnit					(state.getUnit(_playerID, u));
		const Unit & closestUnit				(ourUnit.canHeal() ? state.getClosestOurUnit(_playerID, u) : state.getClosestEnemyUnit(_playerID, u));

		for (IDType m(0); m<moves.numMoves(u); ++m)
		{
			const Move move						(moves.getMove(u, m));
				
			if (move.type() == MoveTypes::ATTACK)
			{
				const Unit & target				(state.getUnit(state.getEnemy(move.player()), move._moveIndex));
				double dpsHPValue 				(target.dpf() / target.currentHP());

				if (dpsHPValue > actionHighestDPS)
				{
					actionHighestDPS = dpsHPValue;
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			else if (move.type() == MoveTypes::HEAL)
			{
				const Unit & target				(state.getUnit(move.player(), move._moveIndex));
				double dpsHPValue 				(target.dpf() / target.currentHP());

				if (dpsHPValue > actionHighestDPS)
				{
					actionHighestDPS = dpsHPValue;
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			else if (move.type() == MoveTypes::MOVE)
			{
				Position ourDest				(ourUnit.x() + Search::Constants::Move_Dir[move._moveIndex][0], 
												 ourUnit.y() + Search::Constants::Move_Dir[move._moveIndex][1]);
				size_t dist						(closestUnit.distSq(ourDest, state.getTime()));

				if (dist > furthestMoveDist)
				{
					furthestMoveDist = dist;
					furthestMoveIndex = m;
				}

				if (dist < closestMoveDist)
				{
					closestMoveDist = dist;
					closestMoveIndex = m;
				}
			}
		}

		// the move we will be returning
		size_t bestMoveIndex(0);

		// if we have an attack move we will use that one
		if (foundAction)
		{
			bestMoveIndex = actionMoveIndex;
		}
		// otherwise use the closest move to the opponent
		else
		{
			// if we are in attack range of the unit, back up
			if (ourUnit.canAttackTarget(closestUnit, state.getTime()))
			{
				bestMoveIndex = furthestMoveIndex;
			}
			// otherwise get back into the fight
			else
			{
				bestMoveIndex = closestMoveIndex;
			}
		}
		
		moveVec.push_back(moves.getMove(u, bestMoveIndex));
	}
}

// TODO: UNTESTED
const MoveTuple Player_KiterDPS::getMoveTuple(GameState & state, const MoveArray & moves)
{
	// the tuple, we will generate this on the fly
	MoveTuple tuple(0);
	
	for (IDType u(0); u<moves.numUnits(); ++u)
	{
		bool foundAction						(false);
		IDType actionMoveIndex					(0);
		IDType furthestMoveIndex				(0);
		size_t furthestMoveDist					(0);
		IDType closestMoveIndex					(0);
		double actionHighestDPS					(0);
		unsigned long long closestMoveDist		(std::numeric_limits<unsigned long long>::max());

		const Unit & ourUnit					(state.getUnit(_playerID, u));
		const Unit & closestUnit				(ourUnit.canHeal() ? state.getClosestOurUnit(_playerID, u) : state.getClosestEnemyUnit(_playerID, u));

		for (IDType m(0); m<moves.numMoves(u); ++m)
		{
			const Move move						(moves.getMove(u, m));
				
			if (move.type() == MoveTypes::ATTACK)
			{
				const Unit & target				(state.getUnit(state.getEnemy(move.player()), move._moveIndex));
				double dpsHPValue 				(target.dpf() / target.currentHP());

				if (dpsHPValue > actionHighestDPS)
				{
					actionHighestDPS = dpsHPValue;
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			else if (move.type() == MoveTypes::HEAL)
			{
				const Unit & target				(state.getUnit(move.player(), move._moveIndex));
				double dpsHPValue 				(target.dpf() / target.currentHP());

				if (dpsHPValue > actionHighestDPS)
				{
					actionHighestDPS = dpsHPValue;
					actionMoveIndex = m;
					foundAction = true;
				}
			}
			else if (move.type() == MoveTypes::MOVE)
			{
				Position ourDest				(ourUnit.x() + Search::Constants::Move_Dir[move._moveIndex][0], 
												 ourUnit.y() + Search::Constants::Move_Dir[move._moveIndex][1]);
				size_t dist						(closestUnit.distSq(ourDest, state.getTime()));

				if (dist > furthestMoveDist)
				{
					furthestMoveDist = dist;
					furthestMoveIndex = m;
				}

				if (dist < closestMoveDist)
				{
					closestMoveDist = dist;
					closestMoveIndex = m;
				}
			}
		}

		// the move we will be returning
		size_t bestMoveIndex(0);

		// if we have an attack move we will use that one
		if (foundAction)
		{
			bestMoveIndex = actionMoveIndex;
		}
		// otherwise use the closest move to the opponent
		else
		{
			// if we are in attack range of the unit, back up
			if (ourUnit.canAttackTarget(closestUnit, state.getTime()))
			{
				bestMoveIndex = furthestMoveIndex;
			}
			// otherwise get back into the fight
			else
			{
				bestMoveIndex = closestMoveIndex;
			}
		}

		// update the tuple calculation
		tuple += bestMoveIndex * moves.getProduct(u);
	}

	return tuple;
}

////////////////////////////////////////
// Random Player
////////////////////////////////////////
Player_Random::Player_Random (const IDType & playerID) 
{
	_playerID = playerID;

	if (Search::Constants::Seed_Player_Random_Time)
	{
		gen.seed(static_cast<unsigned int>(std::time(0)));
	}
}

void Player_Random::getMoves(GameState & state, const MoveArray & moves, std::vector<Move> & moveVec)
{
	for (size_t u(0); u<moves.numUnits(); u++)
	{
		moveVec.push_back(moves.getMove(u, rand() % moves.numMoves(u)));
	}
}

// get desired move tuple for this player's strategy
const MoveTuple Player_Random::getMoveTuple(GameState & state, const MoveArray & moves)
{
	//boost::random::uniform_int_distribution<> dist(0, moves.numMoveTuples() - 1);
		
	return rand() % moves.numMoveTuples();
}
