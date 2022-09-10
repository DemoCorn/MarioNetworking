#include "Mario.h"

#include "TileMap.h"

#include "WalkState.h"
#include "FallState.h"
#include "JumpState.h"
#include "IdleState.h"
#include "Camera.h"
#include "PowerUpManager.h"

void Mario::Load()
{
	mPosition = { 20.0f, 207.0f };

	mStates.push_back(std::make_shared<IdleState>());
	mStates.push_back(std::make_shared<WalkState>());
	mStates.push_back(std::make_shared<JumpState>());
	mStates.push_back(std::make_shared<FallState>());

	X::TextureId TextureId = X::LoadTexture("Mario.png");
	X::TextureId SmallTextureId = X::LoadTexture("SmallMario.png");
	mBigHalfSpriteWidth = (X::GetSpriteWidth(TextureId) / 2.0f) - 3.0f;
	mBigSpriteHeight = X::GetSpriteHeight(TextureId);
	mSmallHalfSpriteWidth = (X::GetSpriteWidth(SmallTextureId) / 2.0f) - 3.0f;
	mSmallSpriteHeight = X::GetSpriteHeight(SmallTextureId);
	
	mPowerUpState = PowerUpState::Small;
	mHalfSpriteWidth = mSmallHalfSpriteWidth;
	mSpriteHeight = mSmallSpriteHeight;

	for (auto&& state : mStates)
	{
		state->Load();
	}

	ChangeState(AnimationState::Fall);
}

void Mario::Unload()
{
	for (auto&& state : mStates)
	{
		state->Unload();
	}

	mStates.clear();
	mCurrentState.reset();
}

void Mario::Update(float deltaTime)
{
	mCurrentState->Update(*this, deltaTime);

	ApplyGravity(deltaTime);
	StopIfOutOfBounds(deltaTime);
}

void Mario::Render()
{
	mCurrentState->Render(*this);
}

X::Math::Rect Mario::GetBoundingBox() const
{
	return {
	mPosition.x - mHalfSpriteWidth, // Left
	mPosition.y - mSpriteHeight, // Top
	mPosition.x + mHalfSpriteWidth, // Right
	mPosition.y, // Bottom
	};
}

void Mario::ChangeState(const AnimationState state)
{
	std::shared_ptr<State> newState = mStates.at(static_cast<int>(state));

	if (mCurrentState == newState)
	{
		return;
	}

	if (mCurrentState != nullptr)
	{
		mCurrentState->Exit(*this);
	}

	mCurrentState = newState;
	if (mCurrentState != nullptr)
	{
		mCurrentState->Enter(*this);
	}
}

void Mario::ApplyGravity(float deltaTime)
{
	if (mJumpHeld) 
	{
		mVelocity.y += (mFallSpeed / 2) * deltaTime;
	}
	else
	{
		mVelocity.y += mFallSpeed * deltaTime;
	}

	mVelocity.y = X::Math::Min(5.0f, mVelocity.y);
}

void Mario::StopIfOutOfBounds(float deltaTime)
{
	X::Math::Vector2 displacement;

	// Get Displacement
	displacement.x = mVelocity.x * deltaTime;
	displacement.y = mVelocity.y;

	const X::Math::Rect currentBoundingBox = GetBoundingBox();

	std::vector<int> BlocksHit;
	std::vector<int> OtherBlocks;

	if (displacement.x > 0.0f)
	{
		X::Math::LineSegment rightEdge{
			currentBoundingBox.max.x + displacement.x,
			currentBoundingBox.min.y,
			currentBoundingBox.max.x + displacement.x,
			currentBoundingBox.max.y
		};

		if (TileMap::Get().IsCollidingWith(rightEdge, OtherBlocks))
		{
			displacement.x = 0.0f;
			mVelocity.x = 0.0f;
		}
	}
	else if (displacement.x < 0.0f)
	{
		X::Math::LineSegment leftEdge{
			currentBoundingBox.min.x + displacement.x,
			currentBoundingBox.min.y,
			currentBoundingBox.min.x + displacement.x,
			currentBoundingBox.max.y
		};

		if (TileMap::Get().IsCollidingWith(leftEdge, OtherBlocks))
		{
			displacement.x = 0.0f;
			mVelocity.x = 0.0f;
		}
	}

	if (displacement.y > 0.0f)
	{
		X::Math::LineSegment bottomEdge{
			currentBoundingBox.min.x,
			currentBoundingBox.max.y + displacement.y,
			currentBoundingBox.max.x,
			currentBoundingBox.max.y + displacement.y
		};

		if (TileMap::Get().IsCollidingWith(bottomEdge, OtherBlocks))
		{
			displacement.y = 0.0f;
			mVelocity.y = 0.0f;
			if (mCurrentState == mStates.at(static_cast<int>(AnimationState::Fall)))
			{
				if (displacement.x != 0.0f)
				{
					ChangeState(AnimationState::Walk);
				}
				else
				{
					ChangeState(AnimationState::Idle);
				}
			}
			
		}
	}

	else if (displacement.y < 0.0f)
	{
		X::Math::LineSegment topEdge{
			currentBoundingBox.min.x,
			currentBoundingBox.min.y + displacement.y,
			currentBoundingBox.max.x,
			currentBoundingBox.min.y + displacement.y
		};

		if (TileMap::Get().IsCollidingWith(topEdge, BlocksHit))
		{
			displacement.y = 0.0f;
			mVelocity.y = 0.0f;
			for (unsigned int i = 0; i < BlocksHit.size(); i++)
			{
				if (TileMap::Get().BlockHit(BlocksHit[i], *this))
				{
					PowerUpManager::Get()->SpawnMushroom(TileMap::Get().PowerUpPosition(BlocksHit[i]));
				}
			}
		}
	}

	if (Camera::Get().ConvertToScreenPosition(mPosition + displacement).x <= mBigHalfSpriteWidth)
	{
		displacement.x = 0;
	}

	mPosition.x += displacement.x;
	mPosition.y += displacement.y;
}

bool Mario::Hit()
{
	if (mPowerUpState == PowerUpState::Big)
	{
		mPowerUpState = PowerUpState::Small;
		return false;
	}
	return true;
}