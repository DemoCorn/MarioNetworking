#include "WalkState.h"
#include "Mario.h"

void WalkState::Load()
{
	mTextureId = X::LoadTexture("Mario_Walking_01.png");
	mTextureIdSmall = X::LoadTexture("SmallMario_Walking_01.png");
}

void WalkState::Update(Mario& owner, float deltaTime)
 {
	if (owner.GetVelocity().y > 0.0f)
	{
		owner.ChangeState(AnimationState::Fall);
	}
	else if (X::IsKeyPressed(X::Keys::SPACE))
	{
		owner.ChangeState(AnimationState::Jump);
	}
	else if (X::IsKeyDown(X::Keys::A))
	{
		auto velocity = owner.GetVelocity();
		if (velocity.x > 0)
		{
			velocity.x += owner.GetSlowSpeed();
		}
		velocity.x += -owner.GetMoveSpeed();

		velocity.x = X::Math::Max(-owner.GetMaxSpeed(), velocity.x);
		owner.SetVelocity(velocity);
		owner.SetFacingLeft(true);
	}
	else if (X::IsKeyDown(X::Keys::D))
	{
		auto velocity = owner.GetVelocity();
		if (velocity.x < 0)
		{
			velocity.x += -owner.GetSlowSpeed();
		}
		velocity.x += owner.GetMoveSpeed();

		velocity.x = X::Math::Min(owner.GetMaxSpeed(), velocity.x);
		owner.SetVelocity(velocity);
		owner.SetFacingLeft(false);
	}
	else
	{
		auto velocity = owner.GetVelocity();
		if (velocity.x > 0)
		{
			velocity.x += owner.GetSlowSpeed();
			if (velocity.x < 0)
			{
				owner.ChangeState(AnimationState::Idle);
			}
		}
		else if (velocity.x < 0)
		{
			velocity.x += -owner.GetSlowSpeed();
			if (velocity.x > 0)
			{
				owner.ChangeState(AnimationState::Idle);
			}
		}
		else
		{
			owner.ChangeState(AnimationState::Idle);
		}
		owner.SetVelocity(velocity);
	}
}