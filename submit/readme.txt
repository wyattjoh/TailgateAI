How to compile program and run:
	Import project in VisualStudio folder (/VisualStudio/UAlbertaBot/VisualStudio), compile and run.

Source Files:
	Source files are in their respective /VisualStudio/*/Source directories, as laid out in the SVN.

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
		- added bool == operator
