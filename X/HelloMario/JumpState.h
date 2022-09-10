#pragma once
#include "State.h"
class JumpState :
    public State
{
public:
    void Load() override;
    void Update(Mario& owner, float deltaTime) override;

    void Enter(Mario& owner) override;
};

