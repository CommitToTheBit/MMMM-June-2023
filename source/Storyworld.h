#pragma once

#include <map>
#include <string>
#include <vector>

struct StoryWorld
{
	struct StoryCharacter
	{
		std::map<std::string, std::string> traits;
	};

	std::string suit;

	std::vector<StoryCharacter> passenger;
	StoryCharacter active, passive;
};

