#pragma once

#include <XEngine.h>
#include "Mario.h"

class TileMap
{
public:
	static void StaticInitialize();
	static void StaticTerminate();
	static TileMap& Get();

	void Load(const std::filesystem::path& mapName, const std::filesystem::path& tilesetName);
	void Unload();
	void Update(float deltaTime);
	void Render();

	bool AddBlock(const X::Math::Vector2 position, const float timeAlive, const int playerID);

	bool IsCollidingWith(const X::Math::LineSegment& lineSegment, std::vector<int>& Blocks) const;
	bool IsCollidingWith(const X::Math::LineSegment& lineSegment) const;

	bool BlockHit(const int nIndex, const Mario mario);
	X::Math::Vector2 PowerUpPosition(const int nIndex) const;

	X::Math::Rect GetBound() const;

private:
	int GetIndex(int column, int row) const;

	void LoadMap(const std::filesystem::path& fileName);
	void LoadTileSet(const std::filesystem::path& fileName);

	std::vector<std::pair<int, float>> mTimedList;
	std::unique_ptr<int[]> mMap = nullptr;
	int mColumns = 0;
	int mRows = 0;

	std::vector<X::TextureId> mTiles;
};

