#include "MicroSearchParameters.h"
#include "Player.h"

using namespace MicroSearch;

// default constructor
MicroSearchParameters::MicroSearchParameters() 
	: _maxPlayer(Search::Players::Player_One)
	, _method(Search::SearchMethods::IDAlphaBeta)
	, _evalMethod(Search::EvaluationMethods::SumHP)
	, _modelSimMethod(Search::PlayerModels::AttackClosest)
	, _usePlayerToMoveMethod(true)
	, _playerToMoveMethod(Search::PlayerToMove::Alternate)
	, _maxDepth(0)
	, _timeLimit(0)
	, _useScriptMoveFirst(false)
	, _scriptMoveFirstMethod(Search::PlayerModels::AttackClosest)
{
	setPlayerModel(Search::Players::Player_One, Search::PlayerModels::None, false);
	setPlayerModel(Search::Players::Player_Two, Search::PlayerModels::None, false);
}

const IDType & MicroSearchParameters::maxPlayer()							const { return _maxPlayer; }
const IDType & MicroSearchParameters::searchMethod()						const { return _method; }
const IDType & MicroSearchParameters::evalMethod()							const { return _evalMethod; }
const IDType & MicroSearchParameters::playerToMoveMethod()					const { return _playerToMoveMethod; }
const IDType & MicroSearchParameters::modelSimMethod()						const { return _modelSimMethod; }
const bool & MicroSearchParameters::useScriptMoveFirst()					const { return _useScriptMoveFirst; }
const bool & MicroSearchParameters::usePlayerToMoveMethod()					const { return _usePlayerToMoveMethod; }
const size_t & MicroSearchParameters::maxDepth()							const { return _maxDepth; }
const size_t & MicroSearchParameters::timeLimit()							const { return _timeLimit; }
const bool MicroSearchParameters::usePlayerModel(const IDType & player)		const { return _usePlayerModel[player]; }
const IDType & MicroSearchParameters::playerModel(const IDType & player)	const { return _playerModel[player]; }
	
void MicroSearchParameters::setMaxPlayer(const IDType & player)									{ _maxPlayer = player; }
void MicroSearchParameters::setSearchMethod(const IDType & method)								{ _method = method; }
void MicroSearchParameters::setEvalMethod(const IDType & eval)									{ _evalMethod = eval; }
void MicroSearchParameters::setPlayerToMoveMethod(const IDType & method)						{ _playerToMoveMethod = method; _usePlayerToMoveMethod = true; }
void MicroSearchParameters::setModelSimMethod(const IDType & method)							{ _modelSimMethod = method; }
void MicroSearchParameters::setScriptMoveFirstMethod(const IDType & method)						{ _scriptMoveFirstMethod = method; _useScriptMoveFirst = true; }
void MicroSearchParameters::setMaxDepth(const size_t & maxDepth)								{ _maxDepth = maxDepth; }
void MicroSearchParameters::setTimeLimit(const size_t & timeLimit)								{ _timeLimit = timeLimit; }
void MicroSearchParameters::setUsePlayerModel(const IDType & player, const bool usePlayerModel)	{ _usePlayerModel[player] = usePlayerModel; }

const boost::shared_ptr<Player> & MicroSearchParameters::getPlayer(const IDType & player) const
{
	return _players[player];
}

void MicroSearchParameters::setPlayerModel(const IDType & player, const IDType & model, const bool & useModel)			
{ 
	_playerModel[player] = model; 
	_usePlayerModel[player] = useModel;
	_players[player] = boost::shared_ptr<Player>(Players::getPlayer(player, model));
}	
