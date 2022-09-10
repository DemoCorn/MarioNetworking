#pragma once
#include "Goomba.h"

class Mushroom : public Goomba
{
public:
	void Load(X::Math::Vector2 position) override;
};

