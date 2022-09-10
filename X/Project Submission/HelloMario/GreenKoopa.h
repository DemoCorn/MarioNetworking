#pragma once
#include "Enemy.h"
class GreenKoopa :
    public Enemy
{
public:
    void virtual Load(X::Math::Vector2 position) override;
    void Update(float deltaTime) override;

    bool GetShelled() { return mShelled; }
    void Hit(X::Math::Vector2 PlayerPosition);

    bool IsMoving() { return mVelocity.x != 0; }

    GreenKoopa& operator=(const GreenKoopa& other) {
        mTextureId = other.mTextureId;
        mPosition = other.mPosition;
        mVelocity = other.mVelocity;
        mActive = other.mActive;
        mHalfSpriteWidth = other.mHalfSpriteWidth;
        mSpriteHeight = other.mSpriteHeight;
        mShelled = other.mShelled;
        mKoopaId = other.mKoopaId;
        mShellId = other.mShellId;

        return *this;
    }

protected:
    bool mShelled = false;
    const float mMoveSpeed = 50.0f;
    X::TextureId mKoopaId;
    X::TextureId mShellId;

};

