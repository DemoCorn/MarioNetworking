#include "PowerUpManager.h"

PowerUpManager* PowerUpManager::Get()
{
	static PowerUpManager sInstance;
	return &sInstance;
}

void PowerUpManager::SpawnMushroom(X::Math::Vector2 position)
{
	push_back(Mushroom());
	at(size() - 1).Load(position);
}

void PowerUpManager::DeleteMushroom(int nIndex)
{
	at(nIndex).Unload();
	erase(begin() + nIndex);
}