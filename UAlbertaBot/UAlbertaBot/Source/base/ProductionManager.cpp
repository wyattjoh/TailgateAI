#include "Common.h"
#include "ProductionManager.h"

#include "..\..\StarcraftBuildOrderSearch\Source\starcraftsearch\ActionSet.hpp"
#include "..\..\StarcraftBuildOrderSearch\Source\starcraftsearch\DFBBStarcraftSearch.hpp"
#include "..\..\StarcraftBuildOrderSearch\Source\starcraftsearch\StarcraftData.hpp"

#define BOADD(N, T, B) for (int i=0; i<N; ++i) { queue.queueAsLowestPriority(MetaType(T), B); }

#define GOAL_ADD(G, M, N) G.push_back(std::pair<MetaType, int>(M, N))

static ZergBuildOrderSearch zerg_build_order_search;

ProductionManager::ProductionManager() 
	: initialBuildSet(false)
	, reservedMinerals(0)
	, reservedGas(0)
	, assignedWorkerForThisBuilding(false)
	, haveLocationForThisBuilding(false)
	, enemyCloakedDetected(false)
	, rushDetected(false)
{
	populateTypeCharMap();

	setBuildOrder(StarcraftBuildOrderSearchManager::Instance().getOpeningBuildOrder());
}

void ProductionManager::setBuildOrder(const std::vector<MetaType> & buildOrder)
{
	// clear the current build order
	queue.clearAll();

	// for each item in the results build order, add it
	for (size_t i(0); i<buildOrder.size(); ++i)
	{
		// queue the item
		queue.queueAsLowestPriority(buildOrder[i], true);
	}
}

void ProductionManager::performBuildOrderSearch(const std::vector< std::pair<MetaType, UnitCountType> > & goal)
{	
	
	if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg) {
		performZergBuildOrderSearch(goal);
	}
	else {
	
		std::vector<MetaType> buildOrder = StarcraftBuildOrderSearchManager::Instance().findBuildOrder(goal);

		// set the build order
		setBuildOrder(buildOrder);
	}
}

void ProductionManager::performZergBuildOrderSearch(const std::vector< std::pair<MetaType, UnitCountType> > & goal)
{
	// clear the current build order
	queue.clearAll();

	if (goal.size() > 0) {
		BOOST_FOREACH (MetaPair order, goal) {
			if (order.first.isUnit() || order.first.isBuilding() || order.first.isUpgrade()) {

				if (order.first.isTech()) {
					if (BWAPI::Broodwar->self()->hasResearched(order.first.techType) ||
						BWAPI::Broodwar->self()->isResearching(order.first.techType)) {
						continue;
					}
				}
				else if (order.first.isUpgrade()) {
					if (BWAPI::Broodwar->self()->isUpgrading(order.first.upgradeType) ||
						BWAPI::Broodwar->self()->getUpgradeLevel(order.first.upgradeType) > 0) {
						continue;
					}
				}

				// Get the dependdancies and prebuild requirements
				std::vector<MetaType> unit_dependancies = zerg_build_order_search.getDependancies(order.first);
				std::vector<MetaType> unit_prebuildrequirements = zerg_build_order_search.getPreBuildRequirements(order.first);

				// Build prerequisites
				if (unit_prebuildrequirements.size() > 0)
					BOOST_FOREACH (MetaType unit_prebuildrequirement, unit_prebuildrequirements) {
						queue.queueAsLowestPriority(unit_prebuildrequirement, true);
					}

				// Build dependencies if not already doing/done so
				if (unit_dependancies.size() > 0)
					BOOST_FOREACH (MetaType unit_dependancy, unit_dependancies) {
						if (unit_dependancy.isUnit() || unit_dependancy.isBuilding()) {
							if (BWAPI::Broodwar->self()->completedUnitCount(unit_dependancy.unitType) == 0 &&
								!BuildingManager::Instance().isBeingBuilt(unit_dependancy.unitType)) {
								queue.queueAsLowestPriority(unit_dependancy, true);
							}
						}
						else if (unit_dependancy.isTech()) {
							if (!BWAPI::Broodwar->self()->hasResearched(unit_dependancy.techType) &&
								!BWAPI::Broodwar->self()->isResearching(unit_dependancy.techType)) {
								queue.queueAsLowestPriority(unit_dependancy, true);
							}
						}
						else if (unit_dependancy.isUpgrade()) {
							if (!BWAPI::Broodwar->self()->isUpgrading(unit_dependancy.upgradeType) &&
								BWAPI::Broodwar->self()->getUpgradeLevel(unit_dependancy.upgradeType) <= 0) {
								queue.queueAsLowestPriority(unit_dependancy, true);
							}
						}
					}

				// Build the actual units too!
				for (int i = 0; i < order.second; i++)
					queue.queueAsLowestPriority(order.first, true);

			}
			else
				BWAPI::Broodwar->printf("ZERGSEARCH: Non unit attempted to add: %s", order.first.getName().c_str());
		}
	}
}

void ProductionManager::update() 
{
	// check the queue for stuff we can build
	manageBuildOrderQueue();

	// if nothing is currently building, get a new goal from the strategy manager
	if (queue.size() == 0 && BWAPI::Broodwar->getFrameCount() > 100)
	{
		BWAPI::Broodwar->drawTextScreen(150, 10, "Nothing left to build, new search!");
		const std::vector< std::pair<MetaType, UnitCountType> > newGoal = StrategyManager::Instance().getBuildOrderGoal();
		
		performBuildOrderSearch(newGoal);
	}

	// detect if there's a build order deadlock once per second
	if ((BWAPI::Broodwar->getFrameCount() % 24 == 0) && detectBuildOrderDeadlock() && BWAPI::Broodwar->getFrameCount() > 2500)
	{
		if (BWAPI::Broodwar->self()->incompleteUnitCount(BWAPI::Broodwar->self()->getRace().getSupplyProvider()) == 0) {
			BWAPI::Broodwar->printf("Supply deadlock detected, building pylon with frame: %d", BWAPI::Broodwar->getFrameCount());
			queue.queueAsHighestPriority(MetaType(BWAPI::Broodwar->self()->getRace().getSupplyProvider()), true);
		}
	}

	// if they have cloaked units get a new goal asap
	if (!enemyCloakedDetected && InformationManager::Instance().enemyHasCloakedUnits() && BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Protoss)
	{
		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Photon_Cannon) < 2)
		{
			queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Protoss_Photon_Cannon), true);
			queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Protoss_Photon_Cannon), true);
		}

		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Forge) == 0)
		{
			queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Protoss_Forge), true);
		}

		BWAPI::Broodwar->printf("Enemy Cloaked Unit Detected!");
		enemyCloakedDetected = true;
	}


//	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(447, 17, "\x07 %d", BuildingManager::Instance().getReservedMinerals());
}

// on unit destroy
void ProductionManager::onUnitDestroy(BWAPI::Unit * unit)
{
	// we don't care if it's not our unit
	if (!unit || unit->getPlayer() != BWAPI::Broodwar->self())
	{
		return;
	}
		
	// if it's a worker or a building, we need to re-search for the current goal
	if ((unit->getType().isWorker() && !WorkerManager::Instance().isWorkerScout(unit)) || unit->getType().isBuilding())
	{
		if (unit->getType() != BWAPI::UnitTypes::Zerg_Drone)
		{
			BWAPI::Broodwar->printf("Critical unit died, re-searching build order: %s", unit->getType().getName().c_str());
			performBuildOrderSearch(StrategyManager::Instance().getBuildOrderGoal());
		}
	}
}

void ProductionManager::manageBuildOrderQueue() 
{
	// if there is nothing in the queue, oh well
	if (queue.isEmpty()) 
	{
		return;
	}

	// the current item to be used
	BuildOrderItem<PRIORITY_TYPE> & currentItem = queue.getHighestPriorityItem();

	// while there is still something left in the queue
	while (!queue.isEmpty()) 
	{
		// this is the unit which can produce the currentItem
		BWAPI::Unit * producer = selectUnitOfType(currentItem.metaType.whatBuilds());

		// check to see if we can make it right now
		bool canMake = canMakeNow(producer, currentItem.metaType);

		// if we try to build too many refineries manually remove it
		if (currentItem.metaType.isRefinery() && (BWAPI::Broodwar->self()->allUnitCount(BWAPI::Broodwar->self()->getRace().getRefinery() >= 3)))
		{
			queue.removeCurrentHighestPriorityItem();
			break;
		}

		// if the next item in the list is a building and we can't yet make it
		if (currentItem.metaType.isBuilding() && !(producer && canMake))
		{
			// construct a temporary building object
			Building b(currentItem.metaType.unitType, BWAPI::Broodwar->self()->getStartLocation());

			// set the producer as the closest worker, but do not set its job yet
			producer = WorkerManager::Instance().getBuilder(b, false);

			// predict the worker movement to that building location
			predictWorkerMovement(b);
		}

		// if we can make the current item
		if (producer && canMake) 
		{
			// create it
			createMetaType(producer, currentItem.metaType);
			assignedWorkerForThisBuilding = false;
			haveLocationForThisBuilding = false;

			// and remove it from the queue
			queue.removeCurrentHighestPriorityItem();

			// don't actually loop around in here
			break;
		}
		// otherwise, if we can skip the current item
		else if (queue.canSkipItem())
		{
			// skip it
			queue.skipItem();

			// and get the next one
			currentItem = queue.getNextHighestPriorityItem();				
		}
		else 
		{
			// so break out
			break;
		}
	}
}

bool ProductionManager::canMakeNow(BWAPI::Unit * producer, MetaType t)
{
	bool canMake = meetsReservedResources(t);
	if (canMake)
	{
		if (t.isUnit())
		{
			canMake = BWAPI::Broodwar->canMake(producer, t.unitType);
		}
		else if (t.isTech())
		{
			canMake = BWAPI::Broodwar->canResearch(producer, t.techType);
		}
		else if (t.isUpgrade())
		{
			canMake = BWAPI::Broodwar->canUpgrade(producer, t.upgradeType);
		}
		else
		{	
			assert(false);
		}
	}

	return canMake;
}

bool ProductionManager::detectBuildOrderDeadlock()
{
	// if the queue is empty there is no deadlock
	if (queue.size() == 0 || BWAPI::Broodwar->self()->supplyTotal() >= 390)
	{
		return false;
	}

	// are any supply providers being built currently
	bool supplyInProgress =		BuildingManager::Instance().isBeingBuilt(BWAPI::Broodwar->self()->getRace().getCenter()) || 
								BuildingManager::Instance().isBeingBuilt(BWAPI::Broodwar->self()->getRace().getSupplyProvider());

	// does the current item being built require more supply
	int supplyCost			= queue.getHighestPriorityItem().metaType.supplyRequired();
	int supplyAvailable		= std::max(0, BWAPI::Broodwar->self()->supplyTotal() - BWAPI::Broodwar->self()->supplyUsed());

	// if we don't have enough supply and none is being built, there's a deadlock
	if ((supplyAvailable < supplyCost) && !supplyInProgress)
	{
		return true;
	}

	return false;
}

// When the next item in the queue is a building, this checks to see if we should move to it
// This function is here as it needs to access prodction manager's reserved resources info
void ProductionManager::predictWorkerMovement(const Building & b)
{
	// get a possible building location for the building
	if (!haveLocationForThisBuilding)
	{
		predictedTilePosition			= BuildingManager::Instance().getBuildingLocation(b);
	}

	if (predictedTilePosition != BWAPI::TilePositions::None)
	{
		haveLocationForThisBuilding		= true;
	}
	else
	{
		return;
	}
	
	// draw a box where the building will be placed
	int x1 = predictedTilePosition.x() * 32;
	int x2 = x1 + (b.type.tileWidth()) * 32;
	int y1 = predictedTilePosition.y() * 32;
	int y2 = y1 + (b.type.tileHeight()) * 32;
	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawBoxMap(x1, y1, x2, y2, BWAPI::Colors::Blue, false);

	// where we want the worker to walk to
	BWAPI::Position walkToPosition		= BWAPI::Position(x1 + (b.type.tileWidth()/2)*32, y1 + (b.type.tileHeight()/2)*32);

	// compute how many resources we need to construct this building
	int mineralsRequired				= std::max(0, b.type.mineralPrice() - getFreeMinerals());
	int gasRequired						= std::max(0, b.type.gasPrice() - getFreeGas());

	// get a candidate worker to move to this location
	BWAPI::Unit * moveWorker			= WorkerManager::Instance().getMoveWorker(walkToPosition);

	// Conditions under which to move the worker: 
	//		- there's a valid worker to move
	//		- we haven't yet assigned a worker to move to this location
	//		- the build position is valid
	//		- we will have the required resources by the time the worker gets there
	if (moveWorker && haveLocationForThisBuilding && !assignedWorkerForThisBuilding && (predictedTilePosition != BWAPI::TilePositions::None) &&
		WorkerManager::Instance().willHaveResources(mineralsRequired, gasRequired, moveWorker->getDistance(walkToPosition)) )
	{
		// we have assigned a worker
		assignedWorkerForThisBuilding = true;

		// tell the worker manager to move this worker
		WorkerManager::Instance().setMoveWorker(mineralsRequired, gasRequired, walkToPosition);
	}
}

void ProductionManager::performCommand(BWAPI::UnitCommandType t) {

	// if it is a cancel construction, it is probably the extractor trick
	if (t == BWAPI::UnitCommandTypes::Cancel_Construction)
	{
		BWAPI::Unit * extractor = NULL;
		BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->self()->getUnits())
		{
			if (unit->getType() == BWAPI::UnitTypes::Zerg_Extractor)
			{
				extractor = unit;
			}
		}

		if (extractor)
		{
			extractor->cancelConstruction();
		}
	}
}

int ProductionManager::getFreeMinerals()
{
	return BWAPI::Broodwar->self()->minerals() - BuildingManager::Instance().getReservedMinerals();
}

int ProductionManager::getFreeGas()
{
	return BWAPI::Broodwar->self()->gas() - BuildingManager::Instance().getReservedGas();
}

// return whether or not we meet resources, including building reserves
bool ProductionManager::meetsReservedResources(MetaType type) 
{
	// return whether or not we meet the resources
	return (type.mineralPrice() <= getFreeMinerals()) && (type.gasPrice() <= getFreeGas());
}

// this function will check to see if all preconditions are met and then create a unit
void ProductionManager::createMetaType(BWAPI::Unit * producer, MetaType t) 
{
	if (!producer)
	{
		return;
	}

	// TODO: special case of evolved zerg buildings needs to be handled

	// if we're dealing with a building
	if (t.isUnit() && t.unitType.isBuilding() 
		&& t.unitType != BWAPI::UnitTypes::Zerg_Lair 
		&& t.unitType != BWAPI::UnitTypes::Zerg_Hive
		&& t.unitType != BWAPI::UnitTypes::Zerg_Greater_Spire)
	{
		// send the building task to the building manager
		BuildingManager::Instance().addBuildingTask(t.unitType, BWAPI::Broodwar->self()->getStartLocation());
	}
	// if we're dealing with a non-building unit
	else if (t.isUnit()) 
	{
		// if the race is zerg, morph the unit
		if (t.unitType.getRace() == BWAPI::Races::Zerg) {
			producer->morph(t.unitType);

		// if not, train the unit
		} else {
			producer->train(t.unitType);
		}
	}
	// if we're dealing with a tech research
	else if (t.isTech())
	{
		producer->research(t.techType);
	}
	else if (t.isUpgrade())
	{
		//Logger::Instance().log("Produce Upgrade: " + t.getName() + "\n");
		producer->upgrade(t.upgradeType);
	}
	else
	{	
		// critical error check
//		assert(false);

		//Logger::Instance().log("createMetaType error: " + t.getName() + "\n");
	}
	
}

// selects a unit of a given type
BWAPI::Unit * ProductionManager::selectUnitOfType(BWAPI::UnitType type, bool leastTrainingTimeRemaining, BWAPI::Position closestTo) {

	// if we have none of the unit type, return NULL right away
	if (BWAPI::Broodwar->self()->completedUnitCount(type) == 0) 
	{
		return NULL;
	}

	BWAPI::Unit * unit = NULL;

	// if we are concerned about the position of the unit, that takes priority
	if (closestTo != BWAPI::Position(0,0)) {

		double minDist(1000000);

		BOOST_FOREACH (BWAPI::Unit * u, BWAPI::Broodwar->self()->getUnits()) {

			if (u->getType() == type) {

				double distance = u->getDistance(closestTo);
				if (!unit || distance < minDist) {
					unit = u;
					minDist = distance;
				}
			}
		}

		// if it is a building and we are worried about selecting the unit with the least
		// amount of training time remaining
	} else if (type.isBuilding() && leastTrainingTimeRemaining) {

		BOOST_FOREACH (BWAPI::Unit * u, BWAPI::Broodwar->self()->getUnits()) {

			if (u->getType() == type && u->isCompleted() && !u->isTraining() && !u->isLifted() &&!u->isUnpowered()) {

				return u;
			}
		}
		// otherwise just return the first unit we come across
	} else {

		BOOST_FOREACH(BWAPI::Unit * u, BWAPI::Broodwar->self()->getUnits()) 
		{
			if (u->getType() == type && u->isCompleted() && u->getHitPoints() > 0 && !u->isLifted() &&!u->isUnpowered()) 
			{
				return u;
			}
		}
	}

	// return what we've found so far
	return NULL;
}

void ProductionManager::onSendText(std::string text)
{
	MetaType typeWanted(BWAPI::UnitTypes::None);
	int numWanted = 0;

	if (text.compare("clear") == 0)
	{
		searchGoal.clear();
	}
	else if (text.compare("search") == 0)
	{
		performBuildOrderSearch(searchGoal);
		searchGoal.clear();
	}
	else if (text[0] >= 'a' && text[0] <= 'z')
	{
		MetaType typeWanted = typeCharMap[text[0]];
		text = text.substr(2,text.size());
		numWanted = atoi(text.c_str());

		searchGoal.push_back(std::pair<MetaType, int>(typeWanted, numWanted));
	}
}

void ProductionManager::populateTypeCharMap()
{
	typeCharMap['p'] = MetaType(BWAPI::UnitTypes::Protoss_Probe);
	typeCharMap['z'] = MetaType(BWAPI::UnitTypes::Protoss_Zealot);
	typeCharMap['d'] = MetaType(BWAPI::UnitTypes::Protoss_Dragoon);
	typeCharMap['t'] = MetaType(BWAPI::UnitTypes::Protoss_Dark_Templar);
	typeCharMap['c'] = MetaType(BWAPI::UnitTypes::Protoss_Corsair);
	typeCharMap['e'] = MetaType(BWAPI::UnitTypes::Protoss_Carrier);
	typeCharMap['h'] = MetaType(BWAPI::UnitTypes::Protoss_High_Templar);
	typeCharMap['n'] = MetaType(BWAPI::UnitTypes::Protoss_Photon_Cannon);
	typeCharMap['a'] = MetaType(BWAPI::UnitTypes::Protoss_Arbiter);
	typeCharMap['r'] = MetaType(BWAPI::UnitTypes::Protoss_Reaver);
	typeCharMap['o'] = MetaType(BWAPI::UnitTypes::Protoss_Observer);
	typeCharMap['s'] = MetaType(BWAPI::UnitTypes::Protoss_Scout);
	typeCharMap['l'] = MetaType(BWAPI::UpgradeTypes::Leg_Enhancements);
	typeCharMap['v'] = MetaType(BWAPI::UpgradeTypes::Singularity_Charge);
}

void ProductionManager::drawProductionInformation(int x, int y)
{
	// fill prod with each unit which is under construction
	std::vector<BWAPI::Unit *> prod;
	BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->isBeingConstructed())
		{
			prod.push_back(unit);
		}
	}
	
	// sort it based on the time it was started
	std::sort(prod.begin(), prod.end(), CompareWhenStarted());

	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y, "\x04 Build Order Info:");
	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, y+20, "\x04UNIT NAME");

	size_t reps = prod.size() < 10 ? prod.size() : 10;

	y += 40;
	int yy = y;

	// for each unit in the queue
	for (size_t i(0); i<reps; i++) {

		std::string prefix = "\x07";

		yy += 10;

		BWAPI::UnitType t = prod[i]->getType();

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x, yy, "%s%s", prefix.c_str(), t.getName().c_str());
		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextScreen(x+150, yy, "%s%6d", prefix.c_str(), prod[i]->getRemainingBuildTime());
	}

	queue.drawQueueInformation(x, yy+10);
}

ProductionManager & ProductionManager::Instance() {

	static ProductionManager instance;
	return instance;
}


ZergBuildOrderSearch::ZergBuildOrderSearch()
{
	// Unit Maps
	static const MetaType map_zerg_zergling(BWAPI::UnitTypes::Zerg_Zergling);
	static const MetaType map_zerg_spawning_pool(BWAPI::UnitTypes::Zerg_Spawning_Pool);
	static const MetaType map_zerg_larva(BWAPI::UnitTypes::Zerg_Larva);
	static const MetaType map_zerg_drone(BWAPI::UnitTypes::Zerg_Drone);
	static const MetaType map_zerg_hatchery(BWAPI::UnitTypes::Zerg_Hatchery);
	static const MetaType map_zerg_hydralisk(BWAPI::UnitTypes::Zerg_Hydralisk);
	static const MetaType map_zerg_hydralisk_den(BWAPI::UnitTypes::Zerg_Hydralisk_Den);
	static const MetaType map_zerg_extractor(BWAPI::UnitTypes::Zerg_Extractor);
	static const MetaType map_zerg_mutalisk(BWAPI::UnitTypes::Zerg_Mutalisk);
	static const MetaType map_zerg_spire(BWAPI::UnitTypes::Zerg_Spire);
	static const MetaType map_zerg_lair(BWAPI::UnitTypes::Zerg_Lair);
	static const MetaType map_zerg_creep_colony(BWAPI::UnitTypes::Zerg_Creep_Colony);
	static const MetaType map_zerg_sunken_colony(BWAPI::UnitTypes::Zerg_Sunken_Colony);

	// Upgrade Maps
	static const MetaType map_grooved_spines(BWAPI::UpgradeTypes::Grooved_Spines);
	static const MetaType map_zerg_missile_attacks(BWAPI::UpgradeTypes::Zerg_Missile_Attacks);
	static const MetaType map_muscular_augments(BWAPI::UpgradeTypes::Muscular_Augments);

	/*************
	 *** Units ***
	 *************/

	// Zerg Sunken Colony
	ZergBuildOrder zerg_sunken_colony(map_zerg_sunken_colony);

	zerg_sunken_colony.prebuildrequirements.push_back(map_zerg_creep_colony);

	build_order.push_back(zerg_sunken_colony);

	// Zerg Creep Colony
	ZergBuildOrder zerg_creep_colony(map_zerg_creep_colony);

	zerg_creep_colony.prebuildrequirements.push_back(map_zerg_drone);

	build_order.push_back(zerg_creep_colony);

	// Zerg Mutalisk
	ZergBuildOrder zerg_mutalisk(map_zerg_mutalisk);

	zerg_mutalisk.dependencies.push_back(map_zerg_spire);
	zerg_mutalisk.dependencies.push_back(map_zerg_extractor);

	build_order.push_back(zerg_mutalisk);

	// Zerg Spire
	ZergBuildOrder zerg_spire(map_zerg_spire);

	zerg_spire.prebuildrequirements.push_back(map_zerg_drone);
	zerg_spire.dependencies.push_back(map_zerg_lair);

	build_order.push_back(zerg_spire);

	// Zerg Lair
	ZergBuildOrder zerg_lair(map_zerg_lair);

	zerg_lair.dependencies.push_back(map_zerg_hatchery);
	zerg_lair.dependencies.push_back(map_zerg_extractor);

	build_order.push_back(zerg_lair);

	// Zerg Hydralisk
	ZergBuildOrder zerg_hydralisk(map_zerg_hydralisk);

	zerg_hydralisk.dependencies.push_back(map_zerg_hydralisk_den);

	build_order.push_back(zerg_hydralisk);

	// Zerg Hydralisk Den
	ZergBuildOrder zerg_hydralisk_den(map_zerg_hydralisk_den);

	zerg_hydralisk_den.prebuildrequirements.push_back(map_zerg_drone);
	zerg_hydralisk_den.dependencies.push_back(map_zerg_spawning_pool);
	zerg_hydralisk_den.dependencies.push_back(map_zerg_extractor);

	build_order.push_back(zerg_hydralisk_den);

	// Zerg Zergling
	ZergBuildOrder zerg_zergling(map_zerg_zergling);

	zerg_zergling.dependencies.push_back(map_zerg_spawning_pool);

	build_order.push_back(zerg_zergling);

	// Zerg Spawning Pool
	ZergBuildOrder zerg_spawning_pool(map_zerg_spawning_pool);

	zerg_spawning_pool.prebuildrequirements.push_back(map_zerg_drone);

	build_order.push_back(zerg_spawning_pool);

	// Zerg Drone
	ZergBuildOrder zerg_drone(map_zerg_drone);

	zerg_drone.dependencies.push_back(map_zerg_hatchery);

	build_order.push_back(zerg_drone);

	/************
	 * Upgrades *
	 ************/

	 // Grooved Spines
	 ZergBuildOrder grooved_spines(map_grooved_spines);

	 grooved_spines.dependencies.push_back(map_zerg_hydralisk_den);
	 grooved_spines.prebuildrequirements.push_back(map_zerg_drone);

	 build_order.push_back(grooved_spines);


}

std::vector<MetaType> ZergBuildOrderSearch::getDependancies(const MetaType & unit)
{
	std::vector<ZergBuildOrder>::iterator unit_order = std::find(build_order.begin(), build_order.end(), unit);

	std::vector<MetaType> dependencies;

	// Check if we found a result
	if (unit_order != build_order.end()) {

		if (unit_order->dependencies.size() > 0) {
			BOOST_FOREACH(MetaType unit_dependancy, unit_order->dependencies) {

				std::vector<MetaType> unit_dependancies = getDependancies(unit_dependancy);

				if (unit_dependancies.size() > 0) {
					BOOST_FOREACH(MetaType sub_unit_dependancy, unit_dependancies) {
						dependencies.push_back(sub_unit_dependancy);
					}
				}

				dependencies.push_back(unit_dependancy);

			}
		}

	}

	return dependencies;
}

std::vector<MetaType> ZergBuildOrderSearch::getPreBuildRequirements(const MetaType & unit)
{
	std::vector<ZergBuildOrder>::iterator unit_order = std::find(build_order.begin(), build_order.end(), unit);

	std::vector<MetaType> prebuildrequirements;

	if (unit_order != build_order.end()) {
		if (unit_order->prebuildrequirements.size() > 0) {
			BOOST_FOREACH(MetaType prebuildrequirement, unit_order->prebuildrequirements) {

				std::vector<MetaType> prebuildrequirements_sub = getPreBuildRequirements(prebuildrequirement);

				if (prebuildrequirements_sub.size() > 0) {
					BOOST_FOREACH(MetaType prebuildrequirement_sub, prebuildrequirements_sub) {
						prebuildrequirements.push_back(prebuildrequirement_sub);
					}
				}

				prebuildrequirements.push_back(prebuildrequirement);

			}
		}
	}

	return prebuildrequirements;
}
