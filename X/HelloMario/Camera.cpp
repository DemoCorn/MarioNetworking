#include "Camera.h"

namespace
{
	std::unique_ptr<Camera> CameraInstance = nullptr;
}

void Camera::StaticInitialize()
{
	XASSERT(CameraInstance == nullptr, "Camera already initialized!");
	CameraInstance = std::make_unique<Camera>();
}
void Camera::StaticTerminate()
{
	CameraInstance.reset();
}
Camera& Camera::Get()
{
	XASSERT(CameraInstance != nullptr, "Camera not yet created! Must call StaticInitialize first!");
	return *CameraInstance;
}

X::Math::Vector2 Camera::ConvertToScreenPosition(const X::Math::Vector2& WorldPosition) const
{
	return WorldPosition - mOffset;
}
X::Math::Vector2 Camera::ConvertToWorldPosition(const X::Math::Vector2& ScreenPosition) const
{
	return ScreenPosition + mOffset;
}

void Camera::SetViewBound(const X::Math::Rect& ViewBound)
{
	mViewBound = ViewBound;

	const float screenWidth = static_cast<float>(X::GetScreenWidth());
	const float screenHeight = static_cast<float>(X::GetScreenHeight());

	if (mViewBound.right - mViewBound.left < screenWidth)
	{
		mViewBound.right = mViewBound.left + screenWidth;
	}
	if (mViewBound.bottom - mViewBound.top < screenHeight)
	{
		mViewBound.bottom = mViewBound.top + screenHeight;
	}


}
void Camera::SetViewPosition(const X::Math::Vector2& ViewPosition)
{
	const float screenWidth = static_cast<float>(X::GetScreenWidth());
	const float screenHeight = static_cast<float>(X::GetScreenHeight());

	const X::Math::Vector2 halfScreenSize{ screenWidth / 2.0f, screenHeight / 2.0f };

	if (mOffset.x >= ViewPosition.x - halfScreenSize.x)
	{
		return;
	}

	mOffset = ViewPosition - halfScreenSize;
	if (mOffset.x < mViewBound.left)
	{
		mOffset.x = mViewBound.left;
	}
	if (mOffset.y < mViewBound.top)
	{
		mOffset.y = mViewBound.top;
	}
	if (mOffset.x > mViewBound.right - screenWidth)
	{
		mOffset.x = mViewBound.right - screenWidth;
	}
	if (mOffset.y > mViewBound.bottom - screenHeight)
	{
		mOffset.y = mViewBound.bottom - screenHeight;
	}
}
