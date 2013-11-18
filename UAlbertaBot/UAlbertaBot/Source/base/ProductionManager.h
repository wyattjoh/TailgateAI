#pragma once

#include <Common.h>
#include "BuildOrderQueue.h"
#include "BuildingManager.h"
#include "StarcraftBuildOrderSearchManager.h"
#include "StrategyManager.h"

typedef unsigned char Action;

class CompareWhenStarted 
{

public:

	CompareWhenStarted() {}

	// the sorting operator
	bool operator() (BWAPI::Unit * u1, BWAPI::Unit * u2) 
	{
		int startedU1 = BWAPI::Broodwar->getFrameCount() - (u1->getType().buildTime() - u1->getRemainingBuildTime());
		int startedU2 = BWAPI::Broodwar->getFrameCount() - (u2->getType().buildTime() - u2->getRemainingBuildTime());
		return startedU1 > startedU2;
    }
};

class ProductionManager 
{
	ProductionManager();

	
	bool						initialBuildSet;

	std::map<char, MetaType>	typeCharMap;
	std::vector< std::pair<MetaType, UnitCountType> > searchGoal;

	bool						assignedWorkerForThisBuilding;
	bool						haveLocationForThisBuilding;
	int							reservedMinerals, reservedGas;
	bool						enemyCloakedDetected;
	bool						rushDetected;

	BWAPI::TilePosition			predictedTilePosition;

	bool						contains(UnitVector & units, BWAPI::Unit * unit);
	void						populateTypeCharMap();

	bool						hasResources(BWAPI::UnitType type);
	bool						canMake(BWAPI::UnitType type);
	bool						hasNumCompletedUnitType(BWAPI::UnitType type, int num);
	bool						meetsReservedResources(MetaType type);
	BWAPI::UnitType				getProducer(MetaType t);

	void						performBuildOrderSearch(const std::vector< std::pair<MetaType, UnitCountType> > & goal);
	void						setBuildOrder(const std::vector<MetaType> & buildOrder);
	void						createMetaType(BWAPI::Unit * producer, MetaType type);
	BWAPI::Unit *				selectUnitOfType(BWAPI::UnitType type, bool leastTrainingTimeRemaining = true, BWAPI::Position closestTo = BWAPI::Position(0,0));
	
	BuildOrderQueue				queue;
	void						manageBuildOrderQueue();
	void						performCommand(BWAPI::UnitCommandType t);
	bool						canMakeNow(BWAPI::Unit * producer, MetaType t);
	void						predictWorkerMovement(const Building & b);

	bool						detectBuildOrderDeadlock();

	int							getFreeMinerals();
	int							getFreeGas();

public:

	static ProductionManager &	Instance();

	void						drawQueueInformation(std::map<BWAPI::UnitType, int> & numUnits, int x, int y, int index);
	void						update();

	void						onUnitMorph(BWAPI::Unit * unit);
	void						onUnitDestroy(BWAPI::Unit * unit);
	void						onSendText(std::string text);

	void						drawProductionInformation(int x, int y);
};
