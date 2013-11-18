#include "SpecProps.h"



namespace sim
{
	const int pixelShift(10);
	const int damageMultipliers[7][6] = 
	{ 
		{0,0,0,0,0,0}, 
		{0,2,3,4,0,0}, 
		{0,4,2,1,0,0},
		{0,4,4,4,0,0},
		{0,4,4,4,0,0},
		{0,0,0,0,0,0}, 
		{0,0,0,0,0,0}
	};

	WeaponProps WeaponProps::props[256];
	UnitProps UnitProps::props[256];
}

using namespace sim;
using namespace BWAPI::UpgradeTypes;

WeaponProps::WeaponProps() :
	rangeUpgrade(BWAPI::WeaponTypes::None),
	speedUpgrade(BWAPI::WeaponTypes::None)
{

}

void WeaponProps::SetType(BWAPI::WeaponType type)
{
	this->type		= type;
	cooldown[0]		= cooldown[1]		= type.damageCooldown();
	maxRange[0]		= maxRange[1]		= type.maxRange() << pixelShift;
}

void WeaponProps::SetRangeUpgrade(BWAPI::UpgradeType upgrade, int maxRange)
{
	rangeUpgrade		= upgrade;
	this->maxRange[1]	= (maxRange << 5) << pixelShift;
}

void WeaponProps::SetSpeedUpgrade(BWAPI::UpgradeType upgrade, int cooldown)
{
	speedUpgrade		= upgrade;
	this->cooldown[1]	= cooldown;
}



UnitProps::UnitProps() : 
	capacityUpgrade(None),
	maxEnergyUpgrade(None),
	sightUpgrade(None),
	extraArmorUpgrade(None),
	speedUpgrade(None)
{
	capacity[0]		= capacity[1]		= 0;
}

void sim::UnitProps::SetType(BWAPI::UnitType type)
{
	this->type		= type;
	maxEnergy[0]	= maxEnergy[1]		= type.maxEnergy();
	sightRange[0]	= sightRange[1]		= type.sightRange() << pixelShift;
	extraArmor[0]	= extraArmor[1]		= 0;
	speed[0]		= speed[1]			= static_cast<int>((1 << pixelShift) * type.topSpeed());
}

void UnitProps::SetSpeedUpgrade(BWAPI::UpgradeType upgrade, double rate)
{
	speedUpgrade				= upgrade;
	speed[1]					= static_cast<int>((1 << pixelShift) * rate);
}

void UnitProps::SetCapacityUpgrade(BWAPI::UpgradeType upgrade, int capacity0, int capacity1)
{
	capacityUpgrade				= upgrade;
	capacity[0]					= capacity0;
	capacity[1]					= capacity1;
}

void UnitProps::SetEnergyUpgrade(BWAPI::UpgradeType upgrade)
{
	maxEnergyUpgrade			= upgrade;
	maxEnergy[1]				= 250;
}

void UnitProps::SetSightUpgrade(BWAPI::UpgradeType upgrade, int range)
{
	sightUpgrade				= upgrade;
	sightRange[1]				= (range << 5) << pixelShift;
}

void UnitProps::SetExtraArmorUpgrade(BWAPI::UpgradeType upgrade, int amount)
{
	extraArmorUpgrade			= upgrade;
	extraArmor[1]				= amount;
}



using namespace BWAPI::WeaponTypes;

void WeaponProps::Init()
{
	for(std::set<BWAPI::WeaponType>::const_iterator it(allWeaponTypes().begin()), end(allWeaponTypes().end()); it!=end; ++it)
	{
		props[it->getID()].SetType(*it);
	}

	props[Gauss_Rifle.getID()			].SetRangeUpgrade(U_238_Shells,			5);	// Terran Marine ground/air attack

	props[Hellfire_Missile_Pack.getID()	].SetRangeUpgrade(Charon_Boosters,		8);	// Terran Goliath air attack

	props[Claws.getID()					].SetSpeedUpgrade(Adrenal_Glands,		6);	// Zerg Zergling ground attack

	props[Needle_Spines.getID()			].SetRangeUpgrade(Grooved_Spines,		5); // Zerg Hydralisk ground/air attack

	props[Phase_Disruptor.getID()		].SetRangeUpgrade(Singularity_Charge,	6);	// Protoss Dragoon ground/air attack
}



using namespace BWAPI::UnitTypes;

void UnitProps::Init()
{
	for(std::set<BWAPI::UnitType>::const_iterator it(allUnitTypes().begin()), end(allUnitTypes().end()); it!=end; ++it)
	{
		props[it->getID()].SetType(*it);
	}

	const double standardSpeed(Terran_SCV.topSpeed());

	props[Terran_Ghost.getID()			].SetEnergyUpgrade(Moebius_Reactor);
	props[Terran_Ghost.getID()			].SetSightUpgrade(Ocular_Implants,			11);

	props[Terran_Medic.getID()			].SetEnergyUpgrade(Caduceus_Reactor);

	props[Terran_Vulture.getID()		].SetSpeedUpgrade(Ion_Thrusters,			standardSpeed * 1.881);

	props[Terran_Wraith.getID()			].SetEnergyUpgrade(Apollo_Reactor);

	props[Terran_Battlecruiser.getID()	].SetEnergyUpgrade(Colossus_Reactor);
	props[Terran_Science_Vessel.getID()	].SetEnergyUpgrade(Titan_Reactor);



	props[Zerg_Zergling.getID()			].SetSpeedUpgrade(Metabolic_Boost,			standardSpeed * 1.615);

	props[Zerg_Hydralisk.getID()		].SetSpeedUpgrade(Muscular_Augments,		standardSpeed * 1.105);

	props[Zerg_Ultralisk.getID()		].SetExtraArmorUpgrade(Chitinous_Plating,	2);
	props[Zerg_Ultralisk.getID()		].SetSpeedUpgrade(Anabolic_Synthesis,		standardSpeed * 1.556);

	props[Zerg_Defiler.getID()			].SetEnergyUpgrade(Metasynaptic_Node);

	props[Zerg_Overlord.getID()			].SetSightUpgrade(Antennae,					11);
	props[Zerg_Overlord.getID()			].SetSpeedUpgrade(Pneumatized_Carapace,		Protoss_Carrier.topSpeed());

	props[Zerg_Queen.getID()			].SetEnergyUpgrade(Gamete_Meiosis);



	props[Protoss_Zealot.getID()		].SetSpeedUpgrade(Leg_Enhancements,			standardSpeed * 1.167);

	props[Protoss_High_Templar.getID()	].SetEnergyUpgrade(Khaydarin_Amulet);

	props[Protoss_Reaver.getID()		].SetCapacityUpgrade(Reaver_Capacity,		5, 10);

	props[Protoss_Dark_Archon.getID()	].SetEnergyUpgrade(Argus_Talisman);

	props[Protoss_Observer.getID()		].SetSightUpgrade(Sensor_Array,				11);
	props[Protoss_Observer.getID()		].SetSpeedUpgrade(Gravitic_Boosters,		Protoss_Corsair.topSpeed());

	props[Protoss_Shuttle.getID()		].SetSpeedUpgrade(Gravitic_Drive,			Protoss_Corsair.topSpeed());

	props[Protoss_Scout.getID()			].SetSightUpgrade(Apial_Sensors,			10);
	props[Protoss_Scout.getID()			].SetSpeedUpgrade(Gravitic_Thrusters,		Protoss_Corsair.topSpeed());

	props[Protoss_Corsair.getID()		].SetEnergyUpgrade(Argus_Jewel);

	props[Protoss_Carrier.getID()		].SetCapacityUpgrade(Carrier_Capacity,		4, 8);

	props[Protoss_Arbiter.getID()		].SetEnergyUpgrade(Khaydarin_Core);
}