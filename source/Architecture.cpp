#include "pch.h"
#include "Architecture.h"

Architecture::Architecture()
{
	m_seed = 0;
}


Architecture::~Architecture()
{

}

void Architecture::Initialize(StoryWorld* world, float seed)
{
	m_world = world;
	m_seed = seed;

	m_storylets = std::vector<Storylet>();

	// Initialize storylet bank...
	std::ifstream f("Storylets.json");
	nlohmann::json data = nlohmann::json::parse(f);

	for (auto datum : data.items())
	{
		if (!data[datum.key()].contains("Beginning"))
			continue;

		if (!data[datum.key()].contains("Middle") || !data[datum.key()]["Middle"].size() > 0)
			continue;

		if (!data[datum.key()].contains("End")) // FIXME: Add nuance here (and further down)...
			continue;

		Storylet storylet = Storylet();
		storylet.progressed = false;

		storylet.beginning = InitializeText(data[datum.key()]["Beginning"]);

		storylet.middle = std::vector<Storylet::Text>(data[datum.key()]["Middle"].size());
		for (int i = 0; i < data[datum.key()]["Middle"].size(); i++)
			storylet.middle[i] = InitializeText(data[datum.key()]["Middle"][i]);

		storylet.end = std::vector<std::vector<Storylet::Text>>(data[datum.key()]["Middle"].size());
		for (int i = 0; i < data[datum.key()]["Middle"].size(); i++)
		{
			int J = (data[datum.key()]["Middle"][i].contains("End")) ? data[datum.key()]["Middle"][i]["End"].size() : 0;

			storylet.end[i] = std::vector<Storylet::Text>(J + 1);
			for (int j = 0; j < J; j++)
				storylet.end[i][j] = InitializeText(data[datum.key()]["Middle"][i]["End"][j]);
			storylet.end[i][J] = InitializeText(data[datum.key()]["End"]); 
		}

		m_storylets.push_back(storylet);
	}
}

Storylet::Text Architecture::InitializeText(nlohmann::json data)
{
	Storylet::Text text = Storylet::Text();
	text.axiom = (data.contains("Axiom")) ? data["Axiom"] : "";
	text.causes = (data.contains("Causes")) ? InitializeCauses(data["Causes"]) : Storylet::Causes();
	text.effects = (data.contains("Effects")) ? InitializeEffects(data["Effects"]) : Storylet::Effects();

	return text;
}

Storylet::Causes Architecture::InitializeCauses(nlohmann::json data)
{
	Storylet::Causes causes = Storylet::Causes();

	if (data.contains("MinPassengers"))
		causes.minPassengers = std::max((int)data["MinPassengers"], causes.minPassengers);

	if (data.contains("MaxPassengers"))
		causes.maxPassengers = std::min((int)data["MaxPassengers"], causes.maxPassengers);

	if (data.contains("ActiveIsPassenger"))
		causes.activeIsPassenger = data["ActiveIsPassenger"];

	if (data.contains("PassiveIsPassenger"))
		causes.passiveIsPassenger = data["PassiveIsPassenger"];

	return causes;
}

Storylet::Effects Architecture::InitializeEffects(nlohmann::json data)
{
	Storylet::Effects effects = Storylet::Effects();

	if (data.contains("ActiveEmbarks"))
		effects.activeEmbarks = data["ActiveEmbarks"];

	if (data.contains("ActiveDisembarks"))
		effects.activeDisembarks = data["ActiveDisembarks"];

	if (data.contains("PassiveEmbarks"))
		effects.passiveEmbarks = data["PassiveEmbarks"];

	if (data.contains("PassiveDisembarks"))
		effects.passiveDisembarks = data["PassiveDisembarks"];

	return effects;
}

Storylet Architecture::SelectBeginning()
{
	srand(m_seed); // FIXME: Hacky, but a good patch in lieu of a better rng implementation?
	m_seed = 2.0f*(std::rand()-RAND_MAX/2)+std::rand()/RAND_MAX;

	if (m_storylets.size() == 0)
		return Storylet();

	// FIXME: Add selection criteria here...
	// FIXME: Identify selection criteria with applicable characters...
	float totalWeight = 0.0f;
	for each (Storylet storylet in m_storylets)
	{
		totalWeight += GetWeight(&storylet.beginning);
	}

	int index = 0;
	float weight = GetRNGRange(0.0f, totalWeight);
	float summedWeight = 0.0f;
	for each (Storylet storylet in m_storylets)
	{
		if (weight > summedWeight + GetWeight(&storylet.beginning))
		{
			summedWeight += GetWeight(&storylet.beginning);
			index++;
		}
		else
		{
			break;
		}
	}

	Storylet storylet = m_storylets[index];

	// FIXME: Assign separately...
	std::vector<std::map<std::string, StoryWorld::StoryCharacter*>> pairings = GetPairings(&storylet.beginning);
	if (pairings.size() > 0)
	{
		std::map<std::string, StoryWorld::StoryCharacter*> pairing = pairings[rand()%pairings.size()];

		storylet.beginning.active = pairing["Active"];
		storylet.beginning.passive = pairing["Passive"];
	}

	return storylet;
}

void Architecture::SelectMiddle(Storylet* storylet)
{
	srand(m_seed); // FIXME: Hacky, but a good patch in lieu of a better rng implementation?
	m_seed = 2.0f*(std::rand()-RAND_MAX/2)+std::rand()/RAND_MAX;

	// FIXME: Add selection criteria here...
	// FIXME: Identify selection criteria with applicable characters...


	while (storylet->middle.size() > 3) // NB: Would this maximum ever be a variable?
		storylet->middle.erase(storylet->middle.begin() + rand()%storylet->middle.size());

	for (int i = 0; i < storylet->middle.size(); i++)
	{
		// FIXME: Add flexibility here!
		storylet->middle[i].active = storylet->beginning.active;
		storylet->middle[i].passive = storylet->beginning.passive;
	}
}

void Architecture::SelectEnd(Storylet* storylet, int choice)
{
	srand(m_seed); // FIXME: Hacky, but a good patch in lieu of a better rng implementation?
	m_seed = 2.0f*(std::rand()-RAND_MAX/2)+std::rand()/RAND_MAX;

	// FIXME: Add selection criteria here...
	// FIXME: Identify selection criteria with applicable characters...

	while (storylet->end[choice].size() > 1) // NB: Would this maximum ever be a variable?
		storylet->end[choice].pop_back();
#
	// FIXME: Add flexibility here!
	storylet->end[choice][0].active = storylet->middle[choice].active;
	storylet->end[choice][0].passive = storylet->middle[choice].passive;
}

std::vector<std::map<std::string, StoryWorld::StoryCharacter*>> Architecture::GetPairings(Storylet::Text* text)
{
	std::vector<std::map<std::string, StoryWorld::StoryCharacter*>> pairings = std::vector<std::map<std::string, StoryWorld::StoryCharacter*>>();
	std::vector<StoryWorld::StoryCharacter*> actives;
	std::vector<StoryWorld::StoryCharacter*> passives;

	if (text->causes.activeIsPassenger)
	{
		// NB: Lacks nuance...
		for (int i = 0; i < m_world->passenger.size(); i++)
		{
			actives.push_back(&m_world->passenger[i]);
		}
	}
	else
	{
		actives.push_back(&m_world->active);
	}

	if (text->causes.passiveIsPassenger)
	{
		// NB: Lacks nuance...
		for (int i = 0; i < m_world->passenger.size(); i++)
		{
			passives.push_back(&m_world->passenger[i]);
		}
	}
	else
	{
		passives.push_back(&m_world->passive);
	}

	for (StoryWorld::StoryCharacter* active : actives)
	{
		for (StoryWorld::StoryCharacter* passive : passives)
		{
			if (active == passive)
				continue;

			std::map<std::string, StoryWorld::StoryCharacter*> pairing = std::map<std::string, StoryWorld::StoryCharacter*>();
			pairing["Active"] = active;
			pairing["Passive"] = passive;
			pairings.push_back(pairing);
		}
	}

	return pairings;
}

float Architecture::GetWeight(Storylet::Text* text)
{
	// STEP 1: Check global conditions...
	if (text->causes.minPassengers > m_world->passenger.size())
		return 0.0f;

	if (text->causes.maxPassengers < m_world->passenger.size())
		return 0.0f;

	// STEP 2: Check *any* characters match...
	if (GetPairings(text).size() == 0)
		return 0.0f;

	return text->weight;
}

float Architecture::GetRNGRange(float a, float b)
{
	return a+(b-a)*std::rand()/RAND_MAX;
}