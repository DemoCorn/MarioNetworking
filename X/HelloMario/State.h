#pragma once

#include <XEngine.h>

class Mario;

class State
{
public:
	virtual ~State() {}

	virtual void Load() {}
	virtual void Unload() {
		mTextureId = 0;
	}
	virtual void Update(Mario& owner, float deltaTime) = 0;
	virtual void Render(Mario& owner);

	virtual void Enter(Mario& owner);
	virtual void Exit(Mario& owner);

protected:
	X::TextureId mTextureId = 0;
	X::TextureId mTextureIdSmall = 0;

	uint32_t mSpriteHeight = 0;
	uint32_t mHalfSpriteHeight = 0;
	uint32_t mHalfSpriteWidth = 0;
};

