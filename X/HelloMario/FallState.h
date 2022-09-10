#pragma once
#include "State.h"
class FallState :
    public State
{
public:
    void Load() override;
    void Update(Mario& owner, float deltaTime) override;
    void Enter(Mario& owner) override;
};

