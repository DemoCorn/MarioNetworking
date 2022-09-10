#include "TileMap.h"
#include "Camera.h"

namespace
{
	const float tileSize = 16.0f;

	std::unique_ptr<TileMap> tileMapInstance;
}

void TileMap::StaticInitialize()
{
	XASSERT(tileMapInstance == nullptr, "TileMap already initialized!");
	tileMapInstance = std::make_unique<TileMap>();
}
void TileMap::StaticTerminate()
{
	tileMapInstance.reset();
}
TileMap& TileMap::Get()
{
	XASSERT(tileMapInstance != nullptr, "TileMap not yet created! Must call StaticInitialize first!");
	return *tileMapInstance;
}

void TileMap::Load(const std::filesystem::path& mapName, const std::filesystem::path& tilesetName)
{
	LoadMap(mapName);
	LoadTileSet(tilesetName);
}
void TileMap::Unload()
{
	mTiles.clear();

	mColumns = 0;
	mRows = 0;

	mMap.reset();
}

void TileMap::Update(float deltaTime)
{
	if (mTimedList.size() != 0)
	{
		for (int i = 0; i < mTimedList.size(); i++)
		{
			mTimedList[i].second -= deltaTime;
			if (mTimedList[i].second <= 0)
			{
				mMap[mTimedList[i].first] = 0;
				mTimedList.erase(mTimedList.begin() + i);
				i--;
			}
		}
	}
}

void TileMap::Render()
{
	for (int y = 0; y < mRows; ++y)
	{
		for (int x = 0; x < mColumns; ++x)
		{
			const int mapIndex = GetIndex(x, y);
			const int tileIndex = mMap[mapIndex];
			const X::TextureId textureId = mTiles.at(tileIndex);

			const X::Math::Vector2 worldPosition = { x * tileSize, y * tileSize };
			const X::Math::Vector2 screenPosition = Camera::Get().ConvertToScreenPosition(worldPosition);

			X::DrawSprite(textureId, screenPosition, X::Pivot::TopLeft);
		}
	}
}

bool TileMap::AddBlock(const X::Math::Vector2 position, const float timeAlive, const int playerID)
{
	const int mapIndex = GetIndex(position.x / tileSize, position.y / tileSize);
	const int tileIndex = mMap[mapIndex];
	if (tileIndex == 0)
	{
		mMap[mapIndex] = 5;
		mTimedList.push_back(std::make_pair(mapIndex, timeAlive));
		return true;
	}
	return false;
}

bool TileMap::IsCollidingWith(const X::Math::LineSegment& lineSegment) const
{
	const int startX = static_cast<int>(lineSegment.from.x / tileSize);
	const int startY = static_cast<int>(lineSegment.from.y / tileSize);
	const int endX = static_cast<int>(lineSegment.to.x / tileSize);
	const int endY = static_cast<int>(lineSegment.to.y / tileSize);

	bool returnval = false;
	for (int y = startY; y <= endY; ++y)
	{
		for (int x = startX; x <= endX; ++x)
		{
			const int index = GetIndex(x, y);
			if (mMap[index] > 4)
			{
				return true;
			}
		}
	}
	return false;
}

bool TileMap::IsCollidingWith(const X::Math::LineSegment& lineSegment, std::vector<int>& Blocks) const
{
	const int startX = static_cast<int>(lineSegment.from.x / tileSize);
	const int startY = static_cast<int>(lineSegment.from.y / tileSize);
	const int endX = static_cast<int>(lineSegment.to.x / tileSize);
	const int endY = static_cast<int>(lineSegment.to.y / tileSize);

	if (startX < 0 || startX >= mColumns ||
		endX < 0 || endX >= mColumns )
	{
		return true;
	}

	bool returnval = false;
	for (int y = startY; y <= endY; ++y)
	{
		for (int x = startX; x <= endX; ++x)
		{
			const int index = GetIndex(x, y);
			if (mMap[index] > 4)
			{
				if (index > 11)
				{
					Blocks.push_back(index);
				}
				returnval = true;
			}
		}
	}

	return returnval;
}

X::Math::Rect TileMap::GetBound() const
{
	return {
		0.0f,
		0.0f,
		mColumns * tileSize,
		mRows * tileSize
	};
}

void TileMap::LoadMap(const std::filesystem::path& fileName)
{
	if (mMap)
	{
		mMap.reset();
	}

	FILE* file = nullptr;
	fopen_s(&file, fileName.u8string().c_str(), "r");

	fscanf_s(file, "Columns: %d\n", &mColumns);
	fscanf_s(file, "Rows: %d\n", &mRows);

	mMap = std::make_unique<int[]>(mColumns * mRows);

	for (int y = 0; y < mRows; ++y)
	{
		for (int x = 0; x < mColumns; ++x)
		{
			const int index = GetIndex(x, y);
			mMap[index] = fgetc(file) - 'A';
		}

		fgetc(file);
	}

	fclose(file);
}
void TileMap::LoadTileSet(const std::filesystem::path& fileName)
{
	mTiles.clear();

	FILE* file = nullptr;
	fopen_s(&file, fileName.u8string().c_str(), "r");

	int count = 0;
	fscanf_s(file, "Count: %d\n", &count);

	for (int i = 0; i < count; ++i)
	{
		char buffer[256];
		fscanf_s(file, "%s\n", buffer, static_cast<int>(std::size(buffer)));

		mTiles.push_back(X::LoadTexture(buffer));
	}

	fclose(file);
}

int TileMap::GetIndex(int column, int row) const
{
	return column + (row * mColumns);
}

bool TileMap::BlockHit(const int nIndex, const Mario mario)
{
	switch (mMap[nIndex])
	{
		case 12:
			mMap[nIndex] = 11;
			return false;

		case 13:
			mMap[nIndex] = 11;
			return true;

		case 14:
			if (mario.GetPowerUpState() != PowerUpState::Small)
			{
				mMap[nIndex] = 0;
			}
			return false;
	}
	return false;
}

X::Math::Vector2 TileMap::PowerUpPosition(const int nIndex) const
{
	X::Math::Vector2 position;

	int x = (nIndex % mColumns);
	int y = (nIndex / mColumns);

	position.y = (y * 16.0f) - 1.0f;
	position.x = x * 16.0f;

	return position;
}