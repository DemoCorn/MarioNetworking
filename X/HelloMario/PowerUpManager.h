#pragma once

#include <XEngine.h>
#include "Mushroom.h"

class PowerUpManager : public std::vector<Mushroom> 
{
public:
	static PowerUpManager* Get();
	void SpawnMushroom(X::Math::Vector2 position);
	void DeleteMushroom(int nIndex);
};

