#include "AlphaBeta.h"

using namespace MicroSearch;

AlphaBeta::AlphaBeta(const MicroSearchParameters & params, TTPtr TT) 
	: _params(params)
	, _currentRootDepth(0)
	, _TT(TT ? TT : TTPtr(new TranspositionTable()))
	, _searchTimer(Timer())
{
	//_allScripts.push_back(PlayerPtr(new Player_AttackClosest	(_params.maxPlayer())));
	//_allScripts.push_back(PlayerPtr(new Player_AttackWeakest	(_params.maxPlayer())));
	//_allScripts.push_back(PlayerPtr(new Player_AttackDPS		(_params.maxPlayer())));
	//_allScripts.push_back(PlayerPtr(new Player_Kiter			(_params.maxPlayer())));
	
	_allScripts.push_back(PlayerPtr(new Player_NOK_AttackDPS	(_params.maxPlayer())));
	//_allScripts.push_back(PlayerPtr(new Player_KiterDPS			(_params.maxPlayer())));
}

void AlphaBeta::doSearch(GameState & initialState)
{
	_searchTimer.start();

	AlphaBetaScore alpha(-10000000, 1000000);
	AlphaBetaScore beta	( 10000000, 1000000);

	AlphaBetaValue val;

	if (_params.searchMethod() == Search::SearchMethods::AlphaBeta)
	{
		val = alphaBeta(initialState, _params.maxDepth(), Search::Players::Player_None, NULL, alpha, beta);
	}
	else if (_params.searchMethod() == Search::SearchMethods::IDAlphaBeta)
	{
		val = IDAlphaBeta(initialState, _params.maxDepth());
	}

	_results.timeElapsed = _searchTimer.getElapsedTimeInMilliSec();
}

AlphaBetaValue AlphaBeta::IDAlphaBeta(GameState & initialState, const size_t & maxDepth)
{
	AlphaBetaValue val;
	_results.nodesExpanded = 0;
	_results.maxDepthReached = 0;

	for (size_t d(1); d < maxDepth; ++d)
	{
		
		AlphaBetaScore alpha(-10000000, 999999);
		AlphaBetaScore beta	( 10000000, 999999);
		
		_results.maxDepthReached = d;
		_currentRootDepth = d;

		// perform ID-AB until time-out
		try
		{
			val = alphaBeta(initialState, d, Search::Players::Player_None, NULL, alpha, beta);

			_results.bestMoveTuple = val.abMove().moveTuple();
			//BWAPI::Broodwar->printf("IDA %d\n", (int)_results.bestMoveTuple);
			_results.abValue = val.score().val();
		}
		// if we do time-out
		catch (int e)
		{
			e += 1;

			// if we didn't finish the first depth, set the move to the best script move
			if (d == 1)
			{
				MoveArray moves;
				const IDType playerToMove(getPlayerToMove(initialState, 1, Search::Players::Player_None, true));
				initialState.generateMoves(moves, playerToMove);
				boost::shared_ptr<Player> bestScript(new Player_NOK_AttackDPS(playerToMove));
				_results.bestMoveTuple = bestScript->getMoveTuple(initialState, moves);
			}

			break;
		}

		long long unsigned nodes = _results.nodesExpanded;
		double ms = _searchTimer.getElapsedTimeInMilliSec();

		//printTTResults();
		//fprintf(stdout, "%s %8d %9d %9d %13.4lf %14llu %12d %12llu %15.2lf\n", "IDA", d, val.score().val(), (int)val.abMove().moveTuple(), ms, nodes, (int)_TT->numFound(), getResults().ttcuts, 1000*nodes/ms);
	}

	return val;
}

// Transposition Table save 
void AlphaBeta::TTsave(	GameState & state, const AlphaBetaScore & value, const AlphaBetaScore & alpha, const AlphaBetaScore & beta, const size_t & depth, 
						const IDType & firstPlayer, const AlphaBetaMove & bestFirstMove, const AlphaBetaMove & bestSecondMove) 
{
	// IF THE DEPTH OF THE ENTRY IS BIGGER THAN CURRENT DEPTH, DO NOTHING
	TTEntry * entry = _TT->lookupScan(state.calculateHash(0), state.calculateHash(1));
	bool valid = entry && entry->isValid();
	size_t edepth = entry ? entry->getDepth() : 0;

	_results.ttSaveAttempts++;
	
	if (valid && (edepth > depth)) 
	{
		return;
	}
	
	int type(TTEntry::NONE);

	if      (value <= alpha) type = TTEntry::UPPER;
	else if (value >= beta)  type = TTEntry::LOWER;
	else                     type = TTEntry::ACCURATE;

	// SAVE A NEW ENTRY IN THE TRANSPOSITION TABLE
	_TT->save(state.calculateHash(0), state.calculateHash(1), value, depth, type, firstPlayer, bestFirstMove, bestSecondMove);
}

// Transposition Table look up + alpha/beta update
TTLookupValue AlphaBeta::TTlookup(const GameState & state, AlphaBetaScore & alpha, AlphaBetaScore & beta, const size_t & depth)
{
	TTEntry * entry = _TT->lookupScan(state.calculateHash(0), state.calculateHash(1));
	if (entry && (entry->getDepth() == depth)) 
	{
		// get the value and type of the entry
		AlphaBetaScore TTvalue = entry->getScore();
		
		// set alpha and beta depending on the type of entry in the TT
		if (entry->getType() == TTEntry::LOWER)
		{
			if (TTvalue > alpha) 
			{
				alpha = TTvalue;
			}
		}
		else if (entry->getType() == TTEntry::UPPER) 
		{
			if (TTvalue < beta)
			{
				beta  = TTvalue;
			}
		} 
		else
		{
			printf("LOL\n");
			alpha = TTvalue;
			beta = TTvalue;
		}
		
		if (alpha >= beta) 
		{
			// this will be a cut
			_results.ttcuts++;
			return TTLookupValue(true, true, entry);
		}
		else
		{
			// found but no cut
			_results.ttFoundNoCut++;
			return TTLookupValue(true, false, entry);
		}
	}
	else if (entry)
	{
		_results.ttFoundLessDepth++;
		return TTLookupValue(true, false, entry);
	}

	return TTLookupValue(false, false, entry);
}

void AlphaBeta::doTupleMoves(GameState & state, MoveArray & moves, const MoveTuple & tuple)
{
	// for each simultaneous move in this tuple
	for (size_t u(0); u<moves.numUnitsInTuple(); ++u)
	{
		Move m = moves.getTupleMove(tuple, u);
		state.makeMove(m, state);
	}
}

const bool AlphaBeta::searchTimeOut()
{
	return (_params.timeLimit() && (_results.nodesExpanded % 50 == 0) && (_searchTimer.getElapsedTimeInMilliSec() >= _params.timeLimit()));
}

const bool AlphaBeta::terminalState(GameState & state, const size_t & depth) const
{
	return (depth <= 0 || (state.winner() != Search::Players::Player_None) || state.isDeadlock());
}

const AlphaBetaMove & AlphaBeta::getAlphaBetaMove(const TTLookupValue & TTval, const IDType & playerToMove) const
{
	const IDType enemyPlayer(getEnemy(playerToMove));

	// if we have a valid first move for this player, use it
	if (TTval.entry()->getBestMove(playerToMove).firstMove().isValid())
	{
		return TTval.entry()->getBestMove(playerToMove).firstMove();
	}
	// otherwise return the response to an opponent move, if it doesn't exist it will just be invalid
	else
	{
		return TTval.entry()->getBestMove(enemyPlayer).secondMove();
	}
}

void AlphaBeta::generateOrderedMoves(GameState & state, MoveArray & moves, const TTLookupValue & TTval, const IDType & playerToMove, const size_t & depth)
{
	// get the array where we will store the moves and clear it
	Array<MoveTuple, Search::Constants::Max_Ordered_Moves> & orderedMoves(_orderedMoves[depth]);
	orderedMoves.clear();

	// if we are using opponent modeling, get the move and then return, we don't want to put any more moves in
	if (_params.usePlayerModel(playerToMove))
	{
		MoveTuple playerModelMove = _params.getPlayer(playerToMove)->getMoveTuple(state, moves);
		orderedMoves.add(playerModelMove);
		return;
	}

	// if there is a transposition table entry for this state
	if (TTval.found())
	{
		// get the abMove we stored for this player
		const AlphaBetaMove & abMove = getAlphaBetaMove(TTval, playerToMove);

		// here we get an incorrect move from the transposition table
		if (abMove.moveTuple() >= moves.numMoveTuples())
		{
			HashType h0 = state.calculateHash(0);
			HashType h1 = state.calculateHash(1);

			MoveArray moves2;
			state.generateMoves(moves2, playerToMove);
			// figure out why
			//fprintf(stderr, "Something very wrong, this tuple (%d) doesn't exist, only (%d) moves\n", (int)abMove.moveTuple(), (int)moves.numMoveTuples());
		}

		_results.ttFoundCheck++;

		// Two checks:
		// 1) Is the move 'valid' ie: was it actually set inside the TT
		// 2) Is it a valid tuple number for this move set? This guards against double
		//    hash collision errors. Even if it is a collision, this is just a move
		//    ordering, so no errors should occur.
		if (abMove.isValid() && (abMove.moveTuple() < moves.numMoveTuples()))
		{
			orderedMoves.add(abMove.moveTuple());
			_results.ttMoveOrders++;
			return;
		}
		else
		{
			_results.ttFoundButNoMove++;
		}
	}

	// if we are using script modeling, insert the script moves we want
	if (_params.useScriptMoveFirst())
	{
		for (size_t s(0); s<_allScripts.size(); s++)
		{
			MoveTuple scriptMove = _allScripts[s]->getMoveTuple(state, moves);

			orderedMoves.addUnique(scriptMove);
		}
	}
}

const MoveTuple AlphaBeta::getNextMoveTuple(const MoveTuple & t, const size_t & depth) const
{
	const Array<MoveTuple, Search::Constants::Max_Ordered_Moves> & orderedMoves(_orderedMoves[depth]);

	// if this move should be from the ordered list, return it from the list
	if (t < orderedMoves.size())
	{
		return orderedMoves[(size_t)t];
	}
	// otherwise return the tuple from outside the list that should be played
	else
	{
		size_t numLessThan(0);
		for (size_t i(0); i<orderedMoves.size(); ++i)
		{
			if (orderedMoves[i] < t)
			{
				numLessThan++;
			}
		}

		return (t - orderedMoves.size()) + numLessThan;
	}
}

const bool AlphaBeta::pruneMove(GameState & state, const IDType & playerToMove, const MoveArray & moves, const MoveTuple & tuple) const
{

	IDType enemy(getEnemy(playerToMove));

	// damage assigned to each enemy unit so far
	int hpRemaining[Search::Constants::Max_Units];
	for (IDType u(0); u<state.numUnits(enemy); ++u)
	{
		hpRemaining[u] = state.getUnit(enemy,u).currentHP();
	}

	// for each unit in the tuple
	for (size_t u(0); u<moves.numUnits(); u++)
	{
		// get its move
		const Move & m(moves.getTupleMove(tuple, u));

		if (m.type() == MoveTypes::ATTACK)
		{
			// if the target unit has already been killed then return prune
			if (hpRemaining[m.index()] <= 0)
			{
				return true;
			}

			hpRemaining[m.index()] -= state.getUnit(playerToMove, u).damage();
		}
	}

	return false;
}

const MoveTuple AlphaBeta::getNumMoveTuples(MoveArray & moves, const TTLookupValue & TTval, const IDType & playerToMove, const size_t & depth) const
{
	// if we are doing opponent modeling, there is just one move to do
	if (_params.usePlayerModel(playerToMove))
	{
		return 1;
	}

	// if there is a transposition table entry for this state
	if (TTval.found())
	{
		// if there was a valid move found with higher depth, just do that one
		const AlphaBetaMove & abMove = getAlphaBetaMove(TTval, playerToMove);
		if ((TTval.entry()->getDepth() >= depth) && abMove.isValid() && (abMove.moveTuple() < moves.numMoveTuples()))
		{
			return 1;
		}
	}

	// otherwise, it's the number of possible moves + number of ordered moves
	return moves.numMoveTuples();
}

const IDType AlphaBeta::getPlayerToMove(GameState & state, const size_t & depth, const IDType & lastPlayerToMove, const bool isFirstSimMove) const
{
	const IDType whoCanMove(state.whoCanMove());

	// if both players can move
	if (whoCanMove == Search::Players::Player_Both)
	{
		// no matter what happens, the 2nd player to move is always the enemy of the first
		if (!isFirstSimMove)
		{
			return getEnemy(lastPlayerToMove);
		}

		// pick the first move based on our policy
		const IDType policy(_params.playerToMoveMethod());
		const IDType maxPlayer(_params.maxPlayer());

		if (policy == Search::PlayerToMove::Alternate)
		{
			return isRoot(depth) ? maxPlayer : getEnemy(lastPlayerToMove);
		}
		else if (policy == Search::PlayerToMove::Not_Alternate)
		{
			return isRoot(depth) ? maxPlayer : lastPlayerToMove;
		}
		else if (policy == Search::PlayerToMove::Random)
		{
			// srand(state.calculateHash(0));
			return isRoot(depth) ? maxPlayer : rand() % 2;
		}

		// we should never get to this state
		assert(false);
		return Search::Players::Player_None;
	}
	else
	{
		return whoCanMove;
	}
}

const bool AlphaBeta::isTranspositionLookupState(GameState & state, const MoveTuple * firstSimMove) const
{
	return !state.bothCanMove() || (state.bothCanMove() && !firstSimMove);
}

AlphaBetaValue AlphaBeta::alphaBeta(GameState & state, size_t depth, const IDType lastPlayerToMove, const MoveTuple * prevSimMove, AlphaBetaScore alpha, AlphaBetaScore beta)
{
	// update statistics
	_results.nodesExpanded++;

	if (searchTimeOut())
	{
		throw 1;
	}

	if (terminalState(state, depth))
	{
		// return the value, but the move will not be valid since none was performed
		AlphaBetaScore evalScore = state.eval(_params.maxPlayer(), _params.evalMethod(), _params.modelSimMethod());
		
		return AlphaBetaValue(AlphaBetaScore(evalScore.val(), state.getNumMovements(_params.maxPlayer()) + evalScore.numMoves() ), AlphaBetaMove(0, false));
	}

	// figure out which player is to move
	const IDType playerToMove(getPlayerToMove(state, depth, lastPlayerToMove, !prevSimMove));

	// is the player to move the max player?
	bool maxPlayer = (playerToMove == _params.maxPlayer());

	// Transposition Table Logic
	TTLookupValue TTval;
	if (isTranspositionLookupState(state, prevSimMove))
	{
		TTval = TTlookup(state, alpha, beta, depth);

		// if this is a TT cut, return the proper value
		if (TTval.cut())
		{
			return AlphaBetaValue(TTval.entry()->getScore(), getAlphaBetaMove(TTval, playerToMove));
		}
	}

	bool bestMoveSet(false);

	// move generation
	MoveArray & moves = _allMoves[depth];
	state.generateMoves(moves, playerToMove);
	generateOrderedMoves(state, moves, TTval, playerToMove, depth);

	// while we have more simultaneous move tuples
	AlphaBetaMove bestMove, bestSimResponse;
	MoveTuple numMoveTuples(getNumMoveTuples(moves, TTval, playerToMove, depth));
	for (MoveTuple t(0); t < numMoveTuples; ++t)
	{
		// get the tuple that will be implemented
		const MoveTuple tuple = getNextMoveTuple(t, depth);

		// the value of the recursive AB we will call
		AlphaBetaValue val;
		
		// generate the child state
		GameState child(state);

		bool firstMove = true;

		// if this is the first player in a simultaneous move state
		if (state.bothCanMove() && !prevSimMove && (depth != 1))
		{
			firstMove = true;
			// don't generate a child yet, just pass on the move we are investigating
			val = alphaBeta(state, depth-1, playerToMove, &tuple, alpha, beta);
		}
		else
		{
			firstMove = false;

			// if this is the 2nd move of a simultaneous move state
			if (prevSimMove)
			{
				// do the previous move tuple selected by the first player to move during this state
				doTupleMoves(child, _allMoves[depth+1], *prevSimMove);
			}

			// do the moves of the current player
			doTupleMoves(child, moves, tuple);
			child.finishedMoving(true);

			// get the alpha beta value
			val = alphaBeta(child, depth-1, playerToMove, NULL, alpha, beta);
		}

		// set alpha or beta based on maxplayer
		if (maxPlayer && (val.score() > alpha)) 
		{
			alpha = val.score();
			bestMove = AlphaBetaMove(tuple, true);
			bestMoveSet = true;

			if (state.bothCanMove() && !prevSimMove)
			{
				bestSimResponse = val.abMove();
			}

			// if this is depth 1 of the first try at depth 1, store the best in results
		}
		else if (!maxPlayer && (val.score() < beta))
		{
			beta = val.score();
			bestMove = AlphaBetaMove(tuple, true);
			bestMoveSet = true;

			if (state.bothCanMove() && prevSimMove)
			{
				bestSimResponse = val.abMove();
			}
		}

		if (alpha.val() == -10000000 && beta.val() == 10000000)
		{
			fprintf(stderr, "\n\nALPHA BETA ERROR, NO VALUE SET\n\n");
		}

		// alpha-beta cut
		if (alpha >= beta) 
		{ 
			break; 
		}
	}
	
	if (isTranspositionLookupState(state, prevSimMove))
	{
		TTsave(state, maxPlayer ? alpha : beta, alpha, beta, depth, playerToMove, bestMove, bestSimResponse);
	}

	return maxPlayer ? AlphaBetaValue(alpha, bestMove) : AlphaBetaValue(beta, bestMove);
}


const SearchResults & AlphaBeta::getResults() const
{
	return _results;
}

const IDType AlphaBeta::getEnemy(const IDType & player) const
{
	return (player + 1) % 2;
}

const bool AlphaBeta::isRoot(const size_t & depth) const
{
	return depth == _currentRootDepth;
}

void AlphaBeta::printTTResults() const
{
	printf("\n");
	printf("Total Usage            %9d\n", (int)_TT->getUsage());
	printf("Save Attempt           %9d\n", (int)_results.ttSaveAttempts);
	printf("   Save Succeed        %9d\n", (int)_TT->numSaves());
	printf("      Save Empty       %9d\n", (int)_TT->saveEmpty);
	printf("      Save Self        %9d\n", (int)_TT->saveOverwriteSelf);
	printf("      Save Other       %9d\n", (int)_TT->saveOverwriteOther);
	printf("Look-Up                %9d\n", (int)_TT->numLookups());
	printf("   Not Found           %9d\n", (int)_TT->numNotFound());
	printf("   Collisions          %9d\n", (int)_TT->numCollisions());
	printf("   Found               %9d\n", (int)_TT->numFound());
	printf("      Less Depth       %9d\n", (int)_results.ttFoundLessDepth);
	printf("      More Depth       %9d\n", ((int)_results.ttFoundCheck + _results.ttcuts));
	printf("         Cut           %9d\n", (int)_results.ttcuts);
	printf("         Move          %9d\n", (int)_results.ttMoveOrders);
	printf("         No Move       %9d\n", (int)_results.ttFoundButNoMove);
	printf("\n");
}
