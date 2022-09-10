#include "GreenKoopa.h"
#include "TileMap.h"

void GreenKoopa::Load(X::Math::Vector2 position)
{
	mTextureId = X::LoadTexture("Green_Koopa.png");
	mKoopaId = mTextureId;
	mShellId = X::LoadTexture("Green_Shell.png");

	mHalfSpriteWidth = (X::GetSpriteWidth(mTextureId) / 2.0f);
	mSpriteHeight = X::GetSpriteHeight(mTextureId);

	mPosition = position;
	mVelocity.x = -mMoveSpeed;
}

void GreenKoopa::Update(float deltaTime) 
{
	if (mActive)
	{
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

void GreenKoopa::Hit(X::Math::Vector2 PlayerPosition)
{
	if (mShelled && mVelocity.x == 0)
	{
		if (PlayerPosition.x >= mPosition.x)
		{
			mVelocity.x = -mMoveSpeed * 2;
		}
		else
		{
			mVelocity.x = mMoveSpeed * 2;
		}
	}
	else if (mShelled)
	{
		mVelocity.x = 0;
	}
	else
	{
		mTextureId = mShellId;
		mShelled = true;
		mVelocity.x = 0;
	}
}