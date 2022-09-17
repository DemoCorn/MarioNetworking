#include "Goomba.h"
#include "TileMap.h"

void Goomba::Load(X::Math::Vector2 position)
{
	mTextureId = X::LoadTexture("Goomba.png");

	mHalfSpriteWidth = (X::GetSpriteWidth(mTextureId) / 2.0f);
	mSpriteHeight = X::GetSpriteHeight(mTextureId);

	mPosition = position;
	mVelocity.x = -50.0f;
}

void Goomba::Load(X::Math::Vector2 position, float timeAlive, int ID, float timeToSpawn)
{
	mTextureId = X::LoadTexture("Goomba.png");

	mHalfSpriteWidth = (X::GetSpriteWidth(mTextureId) / 2.0f);
	mSpriteHeight = X::GetSpriteHeight(mTextureId);

	mPosition = position;
	mVelocity.x = -50.0f;

	isLoading = true;
	activateTimer = timeToSpawn;
	isTimed = true;
	timer = timeAlive;
	playerID = ID;
}

void Goomba::Update(float deltaTime)
{
	if (isLoading)
	{
		currentTimer += deltaTime;
		if (currentTimer > activateTimer)
		{
			isLoading = false;
		}
	}
	if (mActive && !isLoading)
	{
		if (isTimed)
		{
			timer -= deltaTime;
			if (timer <= 0.0f)
			{
				return;
			}
		}
		mVelocity.y += mFallSpeed * deltaTime;
		mVelocity.y = X::Math::Min(5.0f, mVelocity.y);

		bool swapped = false;
		X::Math::Vector2 displacement;

		// Get Displacement
		displacement.x = mVelocity.x * deltaTime;
		displacement.y = mVelocity.y;

		const X::Math::Rect currentBoundingBox = GetBoundingBox();

		if (displacement.x > 0.0f)
		{
			X::Math::LineSegment rightEdge{
				currentBoundingBox.max.x + displacement.x,
				currentBoundingBox.min.y,
				currentBoundingBox.max.x + displacement.x,
				currentBoundingBox.max.y
			};

			if (TileMap::Get().IsCollidingWith(rightEdge))
			{
				displacement.x = -displacement.x;
				mVelocity.x = -mVelocity.x;
				swapped = true;
			}
		}
		if (displacement.x < 0.0f)
		{
			X::Math::LineSegment leftEdge{
				currentBoundingBox.min.x + displacement.x,
				currentBoundingBox.min.y,
				currentBoundingBox.min.x + displacement.x,
				currentBoundingBox.max.y
			};

			if (TileMap::Get().IsCollidingWith(leftEdge))
			{
				if (swapped)
				{
					displacement.x = 0;
					mVelocity.x = -mVelocity.x;
				}
				else
				{
					displacement.x = -displacement.x;
					mVelocity.x = -mVelocity.x;
				}
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

			if (TileMap::Get().IsCollidingWith(bottomEdge))
			{
				displacement.y = 0.0f;
				mVelocity.y = 0.0f;
			}
		}
		mPosition.x += displacement.x;
		mPosition.y += displacement.y;
	}
}