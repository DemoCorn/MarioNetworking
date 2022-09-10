#include <XEngine.h>

#include "Mario.h"
#include "TileMap.h"
#include "Camera.h"
#include "Goomba.h"
#include "PowerUpManager.h"
#include "GreenKoopa.h"

enum class GameState
{
	Start,
	Play,
	Win,
	Lose
};

Mario mario;
std::vector<Goomba> GoombaList;
GreenKoopa ThatOneKoopa;
GameState currentState;
float iFrames = 0.0f;
const float hitIFrames = 1.0f;
const float ShellIFrames = 0.3f;

void GameInit()
{
	Camera::StaticInitialize();

	mario.Load();
	
	for (int i = 0; i < 14; i++) {
		GoombaList.push_back(Goomba());
	}

	GoombaList[0].Load(X::Math::Vector2(23.0f*16.0f-8.0f, 207.0f));
	GoombaList[1].Load(X::Math::Vector2(41.0f*16.0f-8.0f, 207.0f));
	GoombaList[2].Load(X::Math::Vector2(52.0f*16.0f-8.0f, 207.0f));
	GoombaList[3].Load(X::Math::Vector2(54.0f*16.0f, 207.0f));

	GoombaList[4].Load(X::Math::Vector2(81.0f*16.0f-8.0f, 79.0f));
	GoombaList[5].Load(X::Math::Vector2(83.0f*16.0f-8.0f, 79.0f));

	GoombaList[6].Load(X::Math::Vector2(98.0f*16.0f-8.0f, 207.0f));
	GoombaList[7].Load(X::Math::Vector2(100.0f*16.0f, 207.0f));

	ThatOneKoopa.Load(X::Math::Vector2(108.0f * 16.0f, 207.0f));

	GoombaList[8].Load(X::Math::Vector2(115.0f * 16.0f - 8.0f, 207.0f));
	GoombaList[9].Load(X::Math::Vector2(117.0f * 16.0f, 207.0f));

	GoombaList[10].Load(X::Math::Vector2(125.0f * 16.0f - 8.0f, 207.0f));
	GoombaList[11].Load(X::Math::Vector2(127.0f * 16.0f, 207.0f));
	GoombaList[12].Load(X::Math::Vector2(129.0f * 16.0f - 8.0f, 207.0f));
	GoombaList[13].Load(X::Math::Vector2(131.0f * 16.0f, 207.0f));

	GoombaList[12].Load(X::Math::Vector2(175.0f * 16.0f - 8.0f, 207.0f));
	GoombaList[13].Load(X::Math::Vector2(177.0f * 16.0f, 207.0f));

	TileMap::StaticInitialize();
	TileMap::Get().Load("Level_01.txt", "tileset_01.txt");
	X::SetBackgroundColor(X::Color(0.57f, 0.73f, 0.92f, 1.0f));
}

void CollisionCheck()
{
	for (int i = 0; i < (int)PowerUpManager::Get()->size(); i++)
	{
		if (X::Math::Intersect(mario.GetBoundingBox(), PowerUpManager::Get()->at(i).GetBoundingBox()))
		{
			mario.MushroomUp();
			
			PowerUpManager::Get()->DeleteMushroom(i);
			break;
		}
	}
	for (int i = 0; i < (int)GoombaList.size(); i++)
	{
		if (X::Math::Intersect(mario.GetBoundingBox(), GoombaList[i].GetBoundingBox()))
		{
			if (mario.GetVelocity().y > 0.0f)
			{
				GoombaList[i].Unload();
				GoombaList.erase(GoombaList.begin() + i);
				mario.ChangeState(AnimationState::Jump);
			}
			else if (iFrames == 0.0f)
			{
				if (mario.Hit())
				{
					currentState = GameState::Lose;
				}
				else
				{
					iFrames = hitIFrames;
				}
			}

			break;
		}
		if (X::Math::Intersect(ThatOneKoopa.GetBoundingBox(), GoombaList[i].GetBoundingBox()) && ThatOneKoopa.GetShelled() && ThatOneKoopa.GetVelocity().x != 0)
		{
			GoombaList[i].Unload();
			GoombaList.erase(GoombaList.begin() + i);
		}
	}
	if (X::Math::Intersect(mario.GetBoundingBox(), ThatOneKoopa.GetBoundingBox()))
	{
		if (mario.GetVelocity().y > 0.0f or (ThatOneKoopa.GetShelled() and ThatOneKoopa.GetVelocity().x == 0))
		{
			ThatOneKoopa.Hit(mario.GetPosition());
			if (mario.GetVelocity().y > 0.0f)
			{
				mario.ChangeState(AnimationState::Jump);
			}
			else
			{
				iFrames = ShellIFrames;
			}
		}
		else if (iFrames == 0.0f)
		{
			if (mario.Hit())
			{
				currentState = GameState::Lose;
			}
			else
			{
				iFrames = hitIFrames;
			}
		}
	}

}

void GameStateStart(float deltaTime)
{
	X::DrawScreenText("Press Enter To Start!", 65.0f, 100.0f, 30.0f, X::Colors::White);
	if (X::IsKeyPressed(X::Keys::ENTER))
	{
		currentState = GameState::Play;
	}
}

void GameStatePlay(float deltaTime)
{
	TileMap::Get().Update(deltaTime);
	mario.Update(deltaTime);

	ThatOneKoopa.Update(deltaTime);

	for (unsigned int i = 0; i < GoombaList.size(); i++)
	{
		GoombaList[i].Update(deltaTime);
	}

	for (unsigned int i = 0; i < PowerUpManager::Get()->size(); i++)
	{
		PowerUpManager::Get()->at(i).Update(deltaTime);
	}

	Camera::Get().SetViewBound(TileMap::Get().GetBound());
	Camera::Get().SetViewPosition(mario.GetPosition());

	CollisionCheck();

	TileMap::Get().Render();
	mario.Render();

	ThatOneKoopa.Render();

	for (unsigned int i = 0; i < GoombaList.size(); i++)
	{
		GoombaList[i].Render();
	}

	for (unsigned int i = 0; i < PowerUpManager::Get()->size(); i++)
	{
		PowerUpManager::Get()->at(i).Render();
	}

	if (mario.GetPosition().x >= 3184.0f)
	{
		currentState = GameState::Win;
	}
	if (mario.GetPosition().y > X::GetScreenHeight())
	{
		currentState = GameState::Lose;
	}

	iFrames -= deltaTime;
	if (iFrames < 0.0f)
	{
		iFrames = 0.0f;
	}
}


void GameStateLose(float deltaTime) {
	X::DrawScreenText("You Died", 135.0f, 100.0f, 50.0f, X::Colors::White);
}

void GameStateWin(float deltaTime) {
	X::DrawScreenText("You Win", 135.0f, 100.0f, 50.0f, X::Colors::White);
}

bool GameLoop(float deltaTime)
{
	switch (currentState)
	{
	case GameState::Start:
		GameStateStart(deltaTime);
		break;
	case GameState::Play:
		GameStatePlay(deltaTime);
		break;
	case GameState::Win:
		GameStateWin(deltaTime);
		break;
	case GameState::Lose:
		GameStateLose(deltaTime);
		break;
	}

	// Exit condition
	return X::IsKeyPressed(X::Keys::ESCAPE);
}

void GameCleanup()
{
	mario.Unload();
	TileMap::Get().Unload();
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	X::Start("xConfig.json");
	GameInit();

	X::Run(GameLoop);

	GameCleanup();
	X::Stop();

	return 0;
}