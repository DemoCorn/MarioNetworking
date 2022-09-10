#include "IdleState.h"

#include "Mario.h"

void IdleState::Load()
{
	mTextureId = X::LoadTexture("Mario.png");
	mTextureIdSmall = X::LoadTexture("SmallMario.png");
}

void IdleState::Update(Mario& owner, float deltaTime)
{
	if (X::IsKeyPressed(X::Keys::SPACE))
	{
		owner.SetJumpHeld(true);
		owner.ChangeState(AnimationState::Jump);
	}

	else if ((X::IsKeyDown(X::Keys::A)) || (X::IsKeyDown(X::Keys::D)))
	{
		owner.ChangeState(AnimationState::Walk);
	}
}

void IdleState::Enter(Mario& owner)
{
	State::Enter(owner);

	owner.SetVelocity(X::Math::Vector2());
}
