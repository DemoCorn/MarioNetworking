#include "Enemy.h"
#include "Camera.h"

void Enemy::Unload()
{

}

void Enemy::Render()
{
	const X::Math::Vector2 screenPosition = Camera::Get().ConvertToScreenPosition(mPosition);
	mActive = screenPosition.x >= 0 and screenPosition.x <= X::GetScreenWidth();

	const X::Flip flipType = mVelocity.x < 0 ? X::Flip::Horizontal : X::Flip::None;

	if (mActive) {
		X::DrawSprite(mTextureId, screenPosition, X::Pivot::Bottom, flipType);
	}
}