#include "State.h"

#include "Mario.h"
#include "Camera.h"

void State::Render(Mario& owner)
{
	const X::Math::Vector2 screenPosition = Camera::Get().ConvertToScreenPosition(owner.GetPosition());

	const X::Flip flipType = owner.IsFacingLeft() ? X::Flip::Horizontal : X::Flip::None;
	if (owner.GetPowerUpState() == PowerUpState::Big) {
		X::DrawSprite(mTextureId, screenPosition, X::Pivot::Bottom, flipType);
		return;
	}
	X::DrawSprite(mTextureIdSmall, screenPosition, X::Pivot::Bottom, flipType);
}

void State::Enter(Mario& owner)
{

}

void State::Exit(Mario& owner)
{

}