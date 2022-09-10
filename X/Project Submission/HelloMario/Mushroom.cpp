#include "Mushroom.h"

void Mushroom::Load(X::Math::Vector2 position)
{
	Goomba::Load(position);
	mTextureId = X::LoadTexture("Mushroom.png");
}