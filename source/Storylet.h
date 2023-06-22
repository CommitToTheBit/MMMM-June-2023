#pragma once

#include "StoryWorld.h"

#include <map>
#include <string>
#include <vector>

struct Storylet
{
	struct Causes
	{
		int minPassengers;
		int maxPassengers;

		bool activeIsPassenger;

		bool passiveIsPassenger;

		Causes();
		~Causes();
	};

	struct Effects
	{
		bool activeEmbarks;
		bool activeDisembarks;

		bool passiveEmbarks;
		bool passiveDisembarks;

		Effects();
		~Effects();
	};

	struct Text
	{
		std::string axiom;
		Causes causes;
		Effects effects;

		StoryWorld::StoryCharacter* active;
		StoryWorld::StoryCharacter* passive;

		float weight;

		Text();
		~Text();
	};

	Text beginning;
	std::vector<Text> middle;
	std::vector<std::vector<Text>> end;

	bool progressed;

	Storylet();
	~Storylet();
};

