#pragma once

#include "Common.h"
#include <boost/shared_ptr.hpp>

namespace MicroSearch
{
class Player;

typedef boost::shared_ptr<Player> PlayerPtr;

class MicroSearchParameters
{											// DEFAULT				DESCRIPTION
	IDType		_maxPlayer;					// Player_One			The player who will make maximizing moves
	IDType		_method;					// IDAlphaBeta			Algorithm used for searching
	IDType		_evalMethod;				// SumHP				Evaluation function type
	IDType		_modelSimMethod;			// AttackClosest		Policy to use for play-outs
	bool		_usePlayerToMoveMethod;		// True					Whether to use player to move method choice
	IDType		_playerToMoveMethod;		// Alternate			The player to move policy
	size_t		_maxDepth;					// 0					Max depth of search
	size_t		_timeLimit;					// 0					Search time limit. 0 means no time limit
	bool		_useScriptMoveFirst;		// False				Whether we use a script move first
	IDType		_scriptMoveFirstMethod;		// AttackClosest		The script to use when putting move first

																	// DEFAULT				DESCRIPTION
	bool		_usePlayerModel[Search::Constants::Num_Players];	
	IDType		_playerModel[Search::Constants::Num_Players];
	PlayerPtr 	_players[Search::Constants::Num_Players];

public:

	// default constructor
	MicroSearchParameters();

	const IDType & maxPlayer()							const;
	const IDType & searchMethod()						const;
	const IDType & evalMethod()							const;
	const IDType & playerToMoveMethod()					const;
	const IDType & modelSimMethod()						const;
	const IDType & maxPlayerFirst()						const;
	const bool & useScriptMoveFirst()					const;
	const bool & usePlayerToMoveMethod()				const;
	const size_t & maxDepth()							const;
	const size_t & timeLimit()							const;
	const bool usePlayerModel(const IDType & player)	const;
	const IDType & playerModel(const IDType & player)	const;

	const boost::shared_ptr<Player> & getPlayer(const IDType & player) const;
	
	void setMaxPlayer(const IDType & player);
	void setSearchMethod(const IDType & method);
	void setEvalMethod(const IDType & eval);
	void setPlayerToMoveMethod(const IDType & method);
	void setModelSimMethod(const IDType & method);
	void setScriptMoveFirstMethod(const IDType & method);
	void setMaxDepth(const size_t & maxDepth);
	void setTimeLimit(const size_t & timeLimit);
	void setUsePlayerModel(const IDType & player, const bool usePlayerModel);
	void setPlayerModel(const IDType & player, const IDType & model, const bool & useModel);	
};
}