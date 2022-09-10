#pragma once

#include <XEngine.h>

#include "State.h"

enum class AnimationState
{
	None = -1,
	Idle,
	Walk,
	Jump,
	Fall
};

enum class PowerUpState
{
	Small,
	Big
};

class Mario
{
public:
	void Load();
	void Unload();
	void Update(float deltaTime);
	void Render();

	void ChangeState(const AnimationState state);
	void SetPowerUpState(const PowerUpState state) { mPowerUpState = state; }
	void SetVelocity(const X::Math::Vector2& velocity) { mVelocity = velocity; }
	void SetFacingLeft(const bool facingLeft) { mIsFacingLeft = facingLeft; }
	void SetJumpHeld(const bool JumpHeld) { mJumpHeld = JumpHeld; }

	const X::Math::Vector2& GetPosition() const { return mPosition; }
	const X::Math::Vector2& GetVelocity() const { return mVelocity; }
	const PowerUpState GetPowerUpState() const { return mPowerUpState; }
	void MushroomUp() { mPowerUpState = PowerUpState::Big; mSpriteHeight = mBigSpriteHeight; mHalfSpriteWidth = mBigHalfSpriteWidth; }
	bool Hit();

	float GetMoveSpeed() const { return X::IsKeyDown(X::Keys::LSHIFT) ? mMoveSpeed * 2 : mMoveSpeed; }
	float GetSlowSpeed() const { return mSlowSpeed; }
	bool GetFallSpeed() { return mFallSpeed; }
	float GetMaxSpeed() { return X::IsKeyDown(X::Keys::LSHIFT) ? mMaxMoveSpeed * 2 : mMaxMoveSpeed; }
	bool GetJumpHeld() { return mJumpHeld; }

	X::Math::Rect GetBoundingBox() const;
	bool IsFacingLeft() const { return mIsFacingLeft;  }

	

private:
	void ApplyGravity(float deltaTime);
	void StopIfOutOfBounds(float deltaTime);

	X::Math::Vector2 mPosition;
	X::Math::Vector2 mVelocity;

	const float mFallSpeed = 30.0f;
	const float mMaxMoveSpeed = 100.0f;
	const float mMoveSpeed = 1.5f;
	const float mSlowSpeed = -1.5f;
	bool mJumpHeld = false;
	bool mIsFacingLeft = false;

	float mHalfSpriteWidth = 0.0f;
	float mSpriteHeight = 0.0f;
	float mBigHalfSpriteWidth = 0.0f;
	float mBigSpriteHeight = 0.0f;
	float mSmallHalfSpriteWidth = 0.0f;
	float mSmallSpriteHeight = 0.0f;

	PowerUpState mPowerUpState;

	std::shared_ptr<State> mCurrentState;
	std::vector<std::shared_ptr<State>> mStates;
};

