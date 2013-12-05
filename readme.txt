How to compile program and run:
Import project into visual studio and compile

Files and functions we've changed:

StrategyManager.cpp
	-getZergBuildOrderGoal()
	-setStrategy()
	-addStrategies()
	-getTerranBuildOrderGoal() 

StrategyManager.h
	-added enum in class StrategyManager under public:

ProductionManager.cpp
	-ZergBuildOrderSearch()
	-performZergBuildOrderSearch(const std::vector< std::pair <MetaType, UnitCountType>> & goal)
	-getDependancies(const MetaType & unit)
	-getPreBuildRequirements(const MetaType & unit)
	-update()
	-setBuildOrder(const std::vector<MetaType> & buildOrder)
	-onUnitDestroy(BWAPI::Unit * unit)

ProductionManager.h
	-added vector in struct ZergBuildOrder and in struct ZergBuildOrder 
	under private data memebers

MetaType.h
	-in struct MetaData added else{return false;}
	- added bool == operator