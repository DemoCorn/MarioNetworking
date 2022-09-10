#pragma once

#include <XEngine.h>

class Enemy
{
public:
	void virtual Load(X::Math::Vector2 position) = 0;
	void Unload();
	void virtual Update(float deltaTime) = 0;
	void Render();

	X::Math::Rect GetBoundingBox() const {
		return {
		mPosition.x - mHalfSpriteWidth, // Left
		mPosition.y - mSpriteHeight, // Top
		mPosition.x + mHalfSpriteWidth, // Right
		mPosition.y, // Bottom
		};
	}

	void SetPosition(const X::Math::Vector2& position) { mPosition = position; }
	const X::Math::Vector2& GetPosition() const { return mPosition; }
	const X::Math::Vector2& GetVelocity() const { return mVelocity; }

	bool IsActive() { return mActive; }

	Enemy& operator=(const Enemy& other) {
		mTextureId = other.mTextureId;
		mPosition = other.mPosition;
		mVelocity = other.mVelocity;
		mActive = other.mActive;
		mHalfSpriteWidth = other.mHalfSpriteWidth;
		mSpriteHeight = other.mSpriteHeight;

		return *this;
	}

protected:
	X::TextureId mTextureId = 0;

	X::Math::Vector2 mPosition;
	X::Math::Vector2 mVelocity;
	const float mFallSpeed = 30.0f;

	bool mActive = false;

	float mHalfSpriteWidth = 0.0f;
	float mSpriteHeight = 0.0f;
};

