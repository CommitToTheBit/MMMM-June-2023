#include "pch.h"
#include "Storylet.h"

Storylet::Causes::Causes()
{
	minPassengers = 0;
	maxPassengers = INT_MAX;

	activeIsPassenger = false;
	
	passiveIsPassenger = false;
}

Storylet::Causes::~Causes()
{

}

Storylet::Effects::Effects()
{
	activeEmbarks = false;
	activeDisembarks = false;

	passiveEmbarks = false;
	passiveDisembarks = false;
}

Storylet::Effects::~Effects()
{

}

Storylet::Text::Text()
{
	axiom = "";
	causes = Causes();
	effects = Effects();

	active = nullptr;
	passive = nullptr;

	weight = 1.0f;
}

Storylet::Text::~Text()
{

}

Storylet::Storylet()
{
	beginning = Text();
	middle = std::vector<Text>();
	end = std::vector<std::vector<Text>>();

	progressed = false;
}

Storylet::~Storylet()
{

}