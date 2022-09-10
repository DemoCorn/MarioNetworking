#pragma once
#include <XEngine.h>

class Camera
{
public:
	static void StaticInitialize();
	static void StaticTerminate();
	static Camera& Get();

	X::Math::Vector2 ConvertToScreenPosition(const X::Math::Vector2& WorldPosition) const;
	X::Math::Vector2 ConvertToWorldPosition(const X::Math::Vector2& ScreenPosition) const;

	void SetViewBound(const X::Math::Rect& ViewBound);
	void SetViewPosition(const X::Math::Vector2& ViewPosition);

private:

	X::Math::Rect mViewBound{ -FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX };
	X::Math::Vector2 mOffset;
};

