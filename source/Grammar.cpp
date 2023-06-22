#include "pch.h"
#include "Grammar.h"

Grammar::Grammar()
{
	m_seed = 0;
	m_generations = 0;
}


Grammar::~Grammar()
{

}

void Grammar::Initialize(float seed)
{
	m_seed = seed;
	m_generations = 0;

	m_productionRules = std::map<std::string, std::vector<ProductionRuleType>>();

	InitializeCorpus("PronounCorpus.json");

	InitializeCorpus("NamingConventionCorpus.json");
	InitializeCorpus("ForenameCorpus.json");
	InitializeCorpus("PatronymicCorpus.json");
	InitializeCorpus("SurnameCorpus.json");

	InitializeCorpus("MiscCorpus.json");
}

void Grammar::InitializeCorpus(std::string jsonPath)
{
	std::ifstream f(jsonPath);
	nlohmann::json corpus = nlohmann::json::parse(f);

	for (auto letter : corpus.items())
	{
		if (!corpus[letter.key()].contains("productions"))
			continue;

		for (std::string production : corpus[letter.key()]["productions"])
		{
			//m_sentence += production;
			ProductionRuleType productionRule;
			productionRule.production = letter.key();
			productionRule.weight = (corpus[letter.key()].contains("weight")) ? corpus[letter.key()]["weight"] : 1.0f;
			productionRule.recency = 0;

			AddProductionRule(production, productionRule);
		}
	}
}

std::string Grammar::GenerateSentence(std::string axiom, std::string suit, StoryWorld::StoryCharacter* active, StoryWorld::StoryCharacter* passive, bool nested)
{
	srand(m_seed); // FIXME: Hacky, but a good patch in lieu of a better rng implementation?
	m_seed = 2.0f*(std::rand()-RAND_MAX/2)+std::rand()/RAND_MAX;

	if (!nested)
		m_generations++;

	// Handles 'one time only' consistency...
	if (!active)
		active = new StoryWorld::StoryCharacter();

	if (!passive)
		passive = new StoryWorld::StoryCharacter();

	// FIXME: Generalise this to include nested brackets...
	std::string sentence = axiom;

	std::string iteratedSentence, nestedSentence;
	while (sentence.find("{") != -1)
	{
		iteratedSentence = "";
		while (sentence.find("{") != -1)
		{
			// FIXME: HANDLE NESTED BRACKETS!
			// NB: Will need to capitalize anything nested...

			iteratedSentence += sentence.substr(0, sentence.find("{"));
			sentence.erase(sentence.begin(), sentence.begin() + sentence.find("{"));

			int index = FindClosingBracket(sentence);
			if (index < 1)
				continue;

			sentence.erase(sentence.begin(), sentence.begin() + 1); // NB: Must wait for FindClosingBracket call to erase opening bracket...
			index--;

			nestedSentence = GenerateSentence(sentence.substr(0, index), suit, active, passive, true); // NB: Recursive calls like this aren't ideal, but there'll never be too many nested brackets at once...

			if (nestedSentence.find("*") == 0)
				iteratedSentence += (nestedSentence.find("*", 1) == 1) ? GetProductionRule(nestedSentence.substr(2), passive, "**") : GetProductionRule(nestedSentence.substr(1), active, "*");
			else
				iteratedSentence += (nestedSentence == "SUIT") ? suit : GetProductionRule(nestedSentence); // NB: nullptr passed in as 'forgetfulness override'... // NB: "Suit" is currently a special case, but this too can be generalised!

			sentence.erase(sentence.begin(), sentence.begin() + index + 1);
		}
		iteratedSentence += sentence; // NB: Adds the remainder of the sentence, which doesn't need parsed...
		sentence = iteratedSentence;
	}

	if (nested)
	{
		for (int i = 0; i < sentence.length(); i++)
			sentence[i] = std::toupper(sentence[i]);

		return sentence;
	}

	return PostProcessSentence(sentence);
}

std::string Grammar::PostProcessSentence(std::string sentence)
{
	int index;
	std::string iteratedSentence;

	// STEP 1: Remove unnecessary spaces...
	index = sentence.find(" ");
	iteratedSentence = "";
	while (index != -1)
	{
		iteratedSentence += sentence.substr(0, index + 1);
		sentence.erase(sentence.begin(), sentence.begin() + index + 1);
		
		while (sentence.find(" ") == 0)
			sentence.erase(sentence.begin(), sentence.begin() + 1);

		index = sentence.find(" ");
	}
	iteratedSentence += sentence;
	sentence = iteratedSentence;

	if (sentence.find(" ") == 0)
		sentence.erase(sentence.begin(), sentence.begin() + 1);

	if (sentence.find_last_of(" ") == sentence.length() - 1 && sentence.length() > 0)
		sentence.erase(sentence.end() - 1, sentence.end());

	// STEP 2: Capitalise sentence...
	// After the start of the sentence and after every full stop, ensure the next character that can be capitalised *is* capitalised...
	// NB: Is "\n" treated as a single character? If so, should be handled without any special case...
	index = 0;
	while (index < sentence.length() && index != -1)
	{
		if (std::tolower(sentence[index]) == std::toupper(sentence[index]))
		{
			index++;
		}
		else
		{
			sentence[index] = std::toupper(sentence[index]);
			index = sentence.find(".", index + 1);
		}
	}

	// STEP N: Add line breaks...
	// FIXME: Handle this with -1/non-negative int case, with overflow allowed...

	return sentence;
}

void Grammar::AddProductionRule(std::string letter, ProductionRuleType productionRule)
{
	if (!m_productionRules.count(letter))
		m_productionRules.insert({ letter, std::vector<ProductionRuleType>{ productionRule } });
	else
		m_productionRules[letter].push_back(productionRule);
}

std::string Grammar::GetProductionRule(std::string letter, StoryWorld::StoryCharacter* character, std::string consistencyDelimiter, bool generation)
{
	if (!m_productionRules.count(letter))
		return "";

	if (character && character->traits.find(letter) != character->traits.end())
		return character->traits[letter];

	float totalWeight = 0.0f;
	for each (ProductionRuleType productionRule in m_productionRules[letter])
	{
		totalWeight += GetWeight(productionRule, letter);
	}

	int index = 0;
	float weight = GetRNGRange(0.0f, totalWeight);
	float summedWeight = 0.0f;
	for each (ProductionRuleType productionRule in m_productionRules[letter])
	{
		if (weight > summedWeight + GetWeight(productionRule, letter))
		{
			summedWeight += GetWeight(productionRule, letter);
			index++;
		}
		else
		{
			//summedWeight += GetWeight(productionRule, letter);
			break;
		}
	}

	std::string production = m_productionRules[letter][index].production;
	std::string iteratedProduction = "";

	// Parse out "?" delimiters...
	int consistencyIndex = production.find("{");
	if (production.find("?") != -1) // NB: This fixes an infinite while loop, but maybe not robust enough?
	{
		while (consistencyIndex != -1)
		{
			if (production.find("?", consistencyIndex + 1) == consistencyIndex + 1) // FIXME: OOB?
			{
				iteratedProduction += production.substr(0, consistencyIndex) + "{" + consistencyDelimiter;// ONLY STARS BREAK THIS!
				production.erase(production.begin(), production.begin() + consistencyIndex + 2);
				consistencyIndex = 0;
			}

			consistencyIndex = production.find("{", consistencyIndex);
		}
	}
	iteratedProduction += production;
	production = iteratedProduction;

	// Add as consistent feature of world model...
	if (character)
		character->traits[letter] = production;

	// Remember this has been used...
	if (generation)
		m_productionRules[letter][index].recency = m_generations;

	return production;
}

float Grammar::GetWeight(ProductionRuleType productionRule, std::string letter)
{
	float weight = productionRule.weight;

	// STEP 1: Scale by recency...
	std::vector<int> generations = std::vector<int>();
	for (ProductionRuleType rule : m_productionRules[letter])
	{
		generations.push_back(rule.recency);
	}
	std::sort(generations.begin(), generations.end());

	float minimumWeighting = 0.01f; // 0.0f // pow(0.01f, generations.size()); // NB: The most recently-used production rule will be minimumWeighting times as likely to be surfaced as the least recent...
	float exponent = ((float)std::distance(generations.begin(), std::find(generations.begin(), generations.end(), productionRule.recency)))/generations.size();// /generations.size();
	weight *= (productionRule.recency < m_generations) ? pow(minimumWeighting, exponent) : 0.0f; // NB: Explicitely blocks the uses of the same production rule (though not the same word!) in one generation (unless there are no other options)...

	return weight;

}

float Grammar::GetRNGRange(float a, float b)
{
	return a+(b-a)*std::rand()/RAND_MAX;
}

int Grammar::FindClosingBracket(std::string sentence)
{
	if (sentence.find("{") != 0)
		return -1;

	int depth = 1;
	int index = 0;
	int openingIndex, closingIndex;
	while (depth > 0)
	{
		openingIndex = sentence.find("{", index+1);
		closingIndex = sentence.find("}", index+1);

		if (closingIndex == -1)
			return closingIndex;

		if (closingIndex < openingIndex || openingIndex == -1)
		{
			index = closingIndex;
			depth--;
		}
		else
		{
			index = openingIndex;
			depth++;
		}
	}

	return index;
}