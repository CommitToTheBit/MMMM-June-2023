#pragma once

#include "StoryWorld.h"

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

#include <map>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>

class Grammar
{
private:
	struct ProductionRuleType
	{
		std::string production;
		float weight;
		int recency;
	};

public:
	Grammar();
	~Grammar();

	void Initialize(float seed = 0.0f);

	std::string GenerateSentence(std::string axiom, std::string suit, StoryWorld::StoryCharacter* subject = nullptr, StoryWorld::StoryCharacter* object = nullptr, bool nested = false); // NB: Can equally apply this to character initialisation!

private:
	void InitializeCorpus(std::string jsonPath);

	std::string PostProcessSentence(std::string sentence);

	void AddProductionRule(std::string letter, ProductionRuleType productionRule);

	std::string Grammar::GetProductionRule(std::string letter, StoryWorld::StoryCharacter* character = nullptr, std::string consistencyDelimiter = "", bool generation = true);
	float GetWeight(ProductionRuleType productionRule, std::string letter);
	float GetRNGRange(float a = -1.0f, float b = 1.0f);

	int FindClosingBracket(std::string sentence);

private:
	std::map<std::string, std::vector<ProductionRuleType>> m_productionRules;

	float m_seed;
	int m_generations;
};

