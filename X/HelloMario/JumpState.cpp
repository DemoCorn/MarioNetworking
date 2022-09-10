#include "JumpState.h"
#include "Mario.h"

namespace
{
	constexpr float JUMP_SPEED = -5.5f;
}

void JumpState::Load()
{
	mTextureId = X::LoadTexture("Mario_Jump.png");
	mTextureIdSmall = X::LoadTexture("SmallMario_Jump.png");
}

void JumpState::Update(Mario& owner, float deltaTime)
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
		owner.SetVelocity(velocity);
	}


	if (!X::IsKeyDown(X::Keys::SPACE)) 
	{
		owner.SetJumpHeld(false);
	}

	if (owner.GetVelocity().y >= 0.0f)
	{
		owner.ChangeState(AnimationState::Fall);
	}
}

void JumpState::Enter(Mario& owner)
{
	State::Enter(owner);

	owner.SetJumpHeld(true);

	auto velocity = owner.GetVelocity();
	velocity.y = JUMP_SPEED - fabs(velocity.x)/120;

	owner.SetVelocity(velocity);
}