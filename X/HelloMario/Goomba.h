#pragma once
#include "Enemy.h"
class Goomba :
    public Enemy
{
public:
	void virtual Load(X::Math::Vector2 position) override;
	void virtual Load(X::Math::Vector2 position, float timeAlive, int playerID) override;
	void Update(float deltaTime) override;
};

