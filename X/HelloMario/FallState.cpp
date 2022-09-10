#include "FallState.h"
#include "Mario.h"

void FallState::Load()
{
	mTextureId = X::LoadTexture("Mario_Jump.png");
	mTextureIdSmall = X::LoadTexture("SmallMario_Jump.png");
}

void FallState::Update(Mario& owner, float deltaTime)
{
	if (X::IsKeyDown(X::Keys::A))
	{
		auto velocity = owner.GetVelocity();
		velocity.x += -owner.GetMoveSpeed();

		velocity.x = X::Math::Max(-owner.GetMaxSpeed(), velocity.x);
		owner.SetVelocity(velocity);
		owner.SetFacingLeft(true);
	}
	else if (X::IsKeyDown(X::Keys::D))
	{
		auto velocity = owner.GetVelocity();
		velocity.x += owner.GetMoveSpeed();

		velocity.x = X::Math::Min(owner.GetMaxSpeed(), velocity.x);
		owner.SetVelocity(velocity);
		owner.SetFacingLeft(false);
	}
	else
	{
		auto velocity = owner.GetVelocity();
		velocity.x = 0.0f;

		owner.SetVelocity(velocity);
	}
}

void FallState::Enter(Mario& owner)
{
	State::Enter(owner);
	
	owner.SetJumpHeld(false);
}