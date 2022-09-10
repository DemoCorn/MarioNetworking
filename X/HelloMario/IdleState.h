#pragma once

#include <XEngine.h>
#include "State.h"

class IdleState : public State
{
public:
    void Load() override;
    void Update(Mario& owner, float deltaTime) override;
    
    void Enter(Mario& owner) override;
};

