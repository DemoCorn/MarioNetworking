#include <XEngine.h>

#include "Mario.h"
#include "TileMap.h"
#include "Camera.h"
#include "Goomba.h"
#include "PowerUpManager.h"
#include "GreenKoopa.h"
#include "NetworkingComponents.h"

enum class GameState
{
	Start,
	Connect,
	ServerStart,
	ClientStart,
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

bool isServer = false;
bool isClient = false;

using namespace X;

void GameInit()
{
	Camera::StaticInitialize();

	mario.Load();
	
	for (int i = 0; i < 14; i++) {
		GoombaList.push_back(Goomba());
	}

	GoombaList[0].Load(Math::Vector2(23.0f*16.0f-8.0f, 207.0f));
	GoombaList[1].Load(Math::Vector2(41.0f*16.0f-8.0f, 207.0f));
	GoombaList[2].Load(Math::Vector2(52.0f*16.0f-8.0f, 207.0f));
	GoombaList[3].Load(Math::Vector2(54.0f*16.0f, 207.0f));

	GoombaList[4].Load(Math::Vector2(81.0f*16.0f-8.0f, 79.0f));
	GoombaList[5].Load(Math::Vector2(83.0f*16.0f-8.0f, 79.0f));

	GoombaList[6].Load(Math::Vector2(98.0f*16.0f-8.0f, 207.0f));
	GoombaList[7].Load(Math::Vector2(100.0f*16.0f, 207.0f));

	ThatOneKoopa.Load(Math::Vector2(108.0f * 16.0f, 207.0f));

	GoombaList[8].Load(Math::Vector2(115.0f * 16.0f - 8.0f, 207.0f));
	GoombaList[9].Load(Math::Vector2(117.0f * 16.0f, 207.0f));

	GoombaList[10].Load(Math::Vector2(125.0f * 16.0f - 8.0f, 207.0f));
	GoombaList[11].Load(Math::Vector2(127.0f * 16.0f, 207.0f));
	GoombaList[12].Load(Math::Vector2(129.0f * 16.0f - 8.0f, 207.0f));
	GoombaList[13].Load(Math::Vector2(131.0f * 16.0f, 207.0f));

	GoombaList[12].Load(Math::Vector2(175.0f * 16.0f - 8.0f, 207.0f));
	GoombaList[13].Load(Math::Vector2(177.0f * 16.0f, 207.0f));

	TileMap::StaticInitialize();
	TileMap::Get().Load("Level_01.txt", "tileset_01.txt");
	SetBackgroundColor(Color(0.57f, 0.73f, 0.92f, 1.0f));
}

void CollisionCheck()
{
	for (int i = 0; i < (int)PowerUpManager::Get()->size(); i++)
	{
		if (Math::Intersect(mario.GetBoundingBox(), PowerUpManager::Get()->at(i).GetBoundingBox()))
		{
			mario.MushroomUp();
			
			PowerUpManager::Get()->DeleteMushroom(i);
			break;
		}
	}
	for (int i = 0; i < (int)GoombaList.size(); i++)
	{
		if (Math::Intersect(mario.GetBoundingBox(), GoombaList[i].GetBoundingBox()))
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
		if (Math::Intersect(ThatOneKoopa.GetBoundingBox(), GoombaList[i].GetBoundingBox()) && ThatOneKoopa.GetShelled() && ThatOneKoopa.GetVelocity().x != 0)
		{
			GoombaList[i].Unload();
			GoombaList.erase(GoombaList.begin() + i);
		}
	}
	if (Math::Intersect(mario.GetBoundingBox(), ThatOneKoopa.GetBoundingBox()))
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
	DrawScreenText("Press Enter To Start!", 65.0f, 100.0f, 30.0f, Colors::White);
	if (IsKeyPressed(Keys::ENTER))
	{
		currentState = GameState::Connect;
	}
}

void GameStateConnect(float deltaTime)
{
	DrawScreenText("Press Space to Be Server", 65.0f, 70.0f, 30.0f, Colors::White);
	DrawScreenText("Press Enter to Join Server", 65.0f, 100.0f, 30.0f, Colors::White);
	if (IsKeyPressed(Keys::SPACE))
	{
		Server::StaticInitialize();
		if (Server::Get().Startup())
		{
			isServer = true;
			currentState = GameState::ServerStart;
		}
		else
		{
			Server::StaticTerminate();
		}
	}
	if (IsKeyPressed(Keys::ENTER))
	{
		Client::StaticInitialize();
		if (Client::Get().Startup())
		{
			isClient = true;
			currentState = GameState::ClientStart;
		}
		else
		{
			Client::StaticTerminate();
		}
	}
}

void GameStateServerStart(float deltaTime)
{
	Server::Get().Connect();
	std::string message = "Connected Players: " + std::to_string(Server::Get().GetClientCount() + 1) + "/4";
	DrawScreenText(message.c_str(), 65.0f, 70.0f, 30.0f, Colors::White);
	DrawScreenText("Press Enter to Start", 65.0f, 100.0f, 30.0f, Colors::White);
	if (IsKeyPressed(Keys::ENTER)) 
	{
		currentState = GameState::Play;
	}
}

void GameStateClientStart(float deltaTime)
{
	DrawScreenText("Please Wait for Sever to Start", 65.0f, 100.0f, 30.0f, Colors::White);
}

void GameStatePlay(float deltaTime)
{
	TileMap::Get().Update(deltaTime);
	mario.Update(deltaTime);

	if (IsMousePressed(0) || IsMousePressed(1))
	{
		Math::Vector2 ClickPosition(Math::Vector2(GetMouseScreenX(), GetMouseScreenY()));
		ClickPosition = Camera::Get().ConvertToWorldPosition(ClickPosition);
		Math::Vector2 offset(Math::Vector2((int)ClickPosition.x % 16, (int)ClickPosition.y % 16));

		ClickPosition = {
			offset.x >= 8 ? ClickPosition.x + (16.0f - offset.x) : ClickPosition.x - offset.x,
			offset.y >= 8 ? ClickPosition.y + (16.0f - offset.y) : ClickPosition.y - offset.y
		};

		if (IsMousePressed(0))
		{
			GoombaList.push_back(Goomba());
			GoombaList[GoombaList.size() - 1].Load(ClickPosition, 10.0f, 0);
		}
		else if (IsMousePressed(1))
		{
			TileMap::Get().AddBlock(ClickPosition, 10.0f, 0);
		}
	}


	ThatOneKoopa.Update(deltaTime);

	for (unsigned int i = 0; i < GoombaList.size(); i++)
	{
		GoombaList[i].Update(deltaTime);
		if (GoombaList[i].TimeUp())
		{
			GoombaList.erase(GoombaList.begin() + i);
			i--;
		}
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
	if (mario.GetPosition().y > GetScreenHeight())
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
	DrawScreenText("You Died", 135.0f, 100.0f, 50.0f, Colors::White);
}

void GameStateWin(float deltaTime) {
	DrawScreenText("You Win", 135.0f, 100.0f, 50.0f, Colors::White);
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
	case GameState::Connect:
		GameStateConnect(deltaTime);
		break;
	case GameState::ServerStart:
		GameStateServerStart(deltaTime);
		break;
	case GameState::Win:
		GameStateWin(deltaTime);
		break;
	case GameState::Lose:
		GameStateLose(deltaTime);
		break;
	}

	// Exit condition
	return IsKeyPressed(Keys::ESCAPE);
}

void GameCleanup()
{
	if (isServer)
	{
		Server::Get().StaticTerminate();
	}

	if (isClient)
	{
		Client::Get().StaticTerminate();
	}
	mario.Unload();
	TileMap::Get().Unload();
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	std::string name;
	std::cin >> name;

	Start("xConfig.json");
	GameInit();

	Run(GameLoop);

	GameCleanup();
	Stop();

	return 0;
}