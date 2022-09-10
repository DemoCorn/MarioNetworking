#pragma once
#include "State.h"
class WalkState :
    public State
{
    void Load() override;
    void Update(Mario& owner, float deltaTime) override;
};

