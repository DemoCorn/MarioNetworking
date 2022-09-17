#pragma once
#include "Enemy.h"
class Goomba :
    public Enemy
{
public:
	void virtual Load(X::Math::Vector2 position) override;
	void virtual Load(X::Math::Vector2 position, float timeAlive, int playerID, float timeToSpawn) override;
	void Update(float deltaTime) override;

	bool isLoading = false;
	float activateTimer = 0.0f;
	float currentTimer = 0.0f;
};

