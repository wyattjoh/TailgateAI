#include "Simulator.h"

#include <iostream>

namespace BWAPI { Game * Broodwar; }

void SummariseState(const char * description, const sim::State & state);
void SummariseForce(const char * description, const sim::Force & force);

int main()
{
	BWAPI::BWAPI_init();

	sim::ISimulator * simulator(sim::CreateStandardSimulator());

	sim::State state;

	BWAPI::TilePosition posA(0,0);
	BWAPI::TilePosition posB(12,0);

	//state.a.player.SetUpgradeLevel(BWAPI::UpgradeTypes::Metabolic_Boost, 1);
	state.a.AddUnits(BWAPI::UnitTypes::Terran_Marine, 20);

	//state.a.AddUnits(BWAPI::UnitTypes::Zerg_Hydralisk, 10);
	//state.b.AddUnits(BWAPI::UnitTypes::Protoss_Zealot, 10);
	state.b.AddUnits(BWAPI::UnitTypes::Protoss_Zealot, 10);
	//state.b.AddUnits(BWAPI::UnitTypes::Protoss_Dragoon, 4);
	/*state.b.AddUnits(BWAPI::UnitTypes::Protoss_Corsair, 4);
	state.b.AddUnits(BWAPI::UnitTypes::Protoss_Archon, 4);*/

	size_t id(0);
	for(size_t i(0); i<state.a.units.size(); ++i)
	{
		state.a.units[i].SetOriginalUnit((const BWAPI::Unit*)++id);
	}
	for(size_t i(0); i<state.b.units.size(); ++i)
	{
		state.b.units[i].SetOriginalUnit((const BWAPI::Unit*)++id);
	}

	simulator->Reset(state);

	sim::Params params;
	params.forceOrdering	= true;
	simulator->Resolve(params);
	SummariseState("Resolution", simulator->GetState());

	params.killAll			= true;
	simulator->Resolve(params);
	SummariseState("Annihilation", simulator->GetState());

	sim::ISimulator::TargetPairs targetPairs;
	simulator->GetFirstTargets(targetPairs);
	foreach(const sim::ISimulator::TargetPair & pair, targetPairs)
	{
		std::cout << (int)pair.first << " attacks " << (int)pair.second << std::endl;
	}

	return 0;
}

void SummariseState(const char * description, const sim::State & state)
{
	std::cout << description << " (frame " << state.frame << ")" << std::endl;

	SummariseForce("Force A", state.a);
	SummariseForce("Force B", state.b);
	std::cout << std::endl;
}

void SummariseForce(const char * description, const sim::Force & force)
{
	std::cout << "\t" << description;
	if(!force.isThreat) std::cout << " (neutralised)";
	std::cout << std::endl;

	int ore(0), gas(0);
	foreach(const sim::Unit & unit, force.units)
	{
		BWAPI::UnitType type(unit.GetType());
		std::cout << "\t\t" << type.getName() << ": ";
		std::cout << "hp = " << unit.GetHitPoints() << "/" << type.maxHitPoints();
		if(type.maxShields() > 0)
		{
			std::cout << ", shields = " << unit.GetShields() << "/" << type.maxShields();
		}
		std::cout << std::endl;

		int shift(type.isTwoUnitsInOneEgg() ? 0 : 1);
		if(type == BWAPI::UnitTypes::Protoss_Archon)
		{
			type = BWAPI::UnitTypes::Protoss_High_Templar;
			shift = 2;
		}
		else if(type == BWAPI::UnitTypes::Protoss_Dark_Archon)
		{
			type = BWAPI::UnitTypes::Protoss_Dark_Templar;
			shift = 2;
		}
		ore += type.mineralPrice() << shift;
		gas += type.gasPrice() << shift;
	}
	ore = ore >> 1;
	gas = gas >> 1;

	std::cout << "\t\tWorth " << ore << " ore, " << gas << " gas" << std::endl;
}