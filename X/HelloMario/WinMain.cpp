#include <XEngine.h>

#include "Mario.h"
#include "TileMap.h"
#include "Camera.h"
#include "Goomba.h"
#include "PowerUpManager.h"
#include "GreenKoopa.h"
#include "NetworkingComponents.h"
#include "GameState.h"

Mario mario;
std::vector<Goomba> GoombaList;
GreenKoopa ThatOneKoopa;
GameState currentState;
std::vector<int> scores;
float iFrames = 0.0f;
const float hitIFrames = 1.0f;
const float ShellIFrames = 0.3f;

const float blockTime = 5.0f;
const float blockCooldown = 3.0f;
const float goombaTime = 5.0f;
const float goombaCooldown = 7.5f;
const float goombaSpawnTime = 0.5f;

float blockTimer = 0.0f;
float goombaTimer = 0.0f;

std::string ip;
bool isServer = false;
bool isPlayer = false;
int currentPlayer = 0;

using namespace X;

std::vector<std::string> StringSplit(std::string toSplit, char splitter = ' ')
{
	std::stringstream split(toSplit);
	std::string segment;
	std::vector<std::string> seglist;

	while (std::getline(split, segment, splitter))
	{
		seglist.push_back(segment);
	}
	return seglist;
}

void ResolveMessage(std::vector<std::string> message)
{
	if (message.size() == 0)
	{
		return;
	}

	if (message[0] == "M")
	{
		mario.SetPosition({ stof(message[1]), stof(message[2]) });
		mario.SetVelocity({ 0, stof(message[3]) });
		mario.ChangeState(static_cast<AnimationState>(stoi(message[4])));
		mario.SetLeft(stoi(message[5]));
	}
	else if (message[0] == "G")
	{
		GoombaList.push_back(Goomba());
		GoombaList[GoombaList.size() - 1].Load({ stof(message[1]), stof(message[2]) }, blockTime, stoi(message[3]), goombaSpawnTime);
	}
	else if (message[0] == "B")
	{
		TileMap::Get().AddBlock({ stof(message[1]), stof(message[2]) }, blockTime, 0);
	}
	else if (message[0] == "L")
	{
		currentState = GameState::Lose;
	}
	else if (message[0] == "W")
	{
		currentState = GameState::Win;
	}
	else if (message[0] == "P")
	{
		if (!isServer)
		{
			if (Client::Get().GetClientID() == stoi(message[1]))
			{
				isPlayer = true;
			}
			else
			{
				isPlayer = false;
			}

			currentPlayer = stoi(message[1]) + 1;
		}
	}
	else if (message[0] == "S")
	{
		scores.clear();
		for (int i = 1; i < message.size(); i++)
		{
			scores.push_back(stoi(message[i]));
		}
	}
}

int GetID()
{
	if (isServer)
	{
		return 0;
	}
	return Client::Get().GetClientID() + 1;
}

void DrawScore()
{
	float screenWidth = X::GetScreenWidth();
	float spacePerScore = screenWidth / scores.size();

	for (int i = 0; i < scores.size(); i++)
	{
		std::string scoreMessage = "P" + std::to_string(i+1) + ": " + std::to_string(scores[i]);

		if (isServer && i == 0)
		{
			scoreMessage += " (you)";
		}
		else if (!isServer)
		{
			if (Client::Get().GetClientID() + 1 == i)
			{
				scoreMessage += " (you)";
			}
		}

		if (i == currentPlayer)
		{
			DrawScreenText(scoreMessage.c_str(), (spacePerScore / 2) + (spacePerScore * i) - 50.0f, 10.0f, 15.0f, Colors::Yellow);
		}
		else
		{
			DrawScreenText(scoreMessage.c_str(), (spacePerScore / 2) + (spacePerScore * i) - 50.0f, 10.0f, 15.0f, Colors::White);
		}
	}
}

void SendScoreMessage()
{
	std::string message = "S";
	for (int score : scores)
	{
		message.append(" " + std::to_string(score));
	}
	NetworkingComponent::Send(message, isServer);
}

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

void Reset()
{
	Camera::Get().Reset();

	mario.SetPosition({ 20.0f, 207.0f });
	mario.SetVelocity({ 0.0f,0.0f });
	mario.SetLeft(false);
	mario.SetPowerUpState(PowerUpState::Small);

	TileMap::Get().Load("Level_01.txt", "tileset_01.txt");
	SetBackgroundColor(Color(0.57f, 0.73f, 0.92f, 1.0f));

	for (int i = 0; i < GoombaList.size(); i++) {
		GoombaList[i].Unload();
	}
	GoombaList.clear();
	for (int i = 0; i < 14; i++) {
		GoombaList.push_back(Goomba());
	}

	GoombaList[0].Load(Math::Vector2(23.0f * 16.0f - 8.0f, 207.0f));
	GoombaList[1].Load(Math::Vector2(41.0f * 16.0f - 8.0f, 207.0f));
	GoombaList[2].Load(Math::Vector2(52.0f * 16.0f - 8.0f, 207.0f));
	GoombaList[3].Load(Math::Vector2(54.0f * 16.0f, 207.0f));

	GoombaList[4].Load(Math::Vector2(81.0f * 16.0f - 8.0f, 79.0f));
	GoombaList[5].Load(Math::Vector2(83.0f * 16.0f - 8.0f, 79.0f));

	GoombaList[6].Load(Math::Vector2(98.0f * 16.0f - 8.0f, 207.0f));
	GoombaList[7].Load(Math::Vector2(100.0f * 16.0f, 207.0f));

	ThatOneKoopa.Load(Math::Vector2(108.0f * 16.0f, 207.0f));

	GoombaList[8].Load(Math::Vector2(115.0f * 16.0f - 8.0f, 207.0f));
	GoombaList[9].Load(Math::Vector2(117.0f * 16.0f, 207.0f));

	GoombaList[10].Load(Math::Vector2(125.0f * 16.0f - 8.0f, 207.0f));
	GoombaList[11].Load(Math::Vector2(127.0f * 16.0f, 207.0f));
	GoombaList[12].Load(Math::Vector2(129.0f * 16.0f - 8.0f, 207.0f));
	GoombaList[13].Load(Math::Vector2(131.0f * 16.0f, 207.0f));

	GoombaList[12].Load(Math::Vector2(175.0f * 16.0f - 8.0f, 207.0f));
	GoombaList[13].Load(Math::Vector2(177.0f * 16.0f, 207.0f));
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
		if (!GoombaList[i].isLoading)
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
					if (mario.Hit() && isPlayer)
					{
						currentState = GameState::Lose;
						if (GoombaList[i].GetPlayerID() != -1)
						{
							scores[GoombaList[i].GetPlayerID()]++;
							SendScoreMessage();
						}
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
	}
	if (Math::Intersect(mario.GetBoundingBox(), ThatOneKoopa.GetBoundingBox()))
	{
		if (mario.GetVelocity().y > 0.0f || (ThatOneKoopa.GetShelled() && ThatOneKoopa.GetVelocity().x == 0))
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
			if (mario.Hit() && isPlayer)
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
	DrawScreenText("Press Space to Be Server", 35.0f, 70.0f, 30.0f, Colors::White);
	DrawScreenText("Press Enter to Join Server", 25.0f, 100.0f, 30.0f, Colors::White);
	if (IsKeyPressed(Keys::SPACE))
	{
		Server::StaticInitialize();
		if (Server::Get().Startup())
		{
			isServer = true;
			isPlayer = true;
			Server::Get().Set(&currentState);
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
		currentState = GameState::ClientStart;
	}
}

void GameStateServerStart(float deltaTime)
{
	std::string message = "Connected Players: " + std::to_string(Server::Get().GetClientCount() + 1) + "/4";
	DrawScreenText(message.c_str(), 45.0f, 70.0f, 30.0f, Colors::White);
	DrawScreenText("Press Enter to Start", 45.0f, 100.0f, 30.0f, Colors::White);
	if (IsKeyPressed(Keys::ENTER)) 
	{
		for (int i = 0; i < Server::Get().GetClientCount() + 1; i++) { scores.push_back(0); }
		Server::Get().Send("Play");
		SendScoreMessage();

		currentState = GameState::Play;
	}
}

void GameStateClientStart(float deltaTime)
{
	DrawScreenText("Enter Server IPV4", 95.0f, 30.0f, 30.0f, Colors::White);
	DrawScreenText("Shift to go back", 95.0f, 60.0f, 30.0f, Colors::White);
	DrawScreenText("Enter to confirm", 95.0f, 90.0f, 30.0f, Colors::White);
	DrawScreenText(ip.c_str(), 95.0f, 120.0f, 30.0f, Colors::Yellow);
	if (IsKeyPressed(Keys::LSHIFT))
	{
		Client::StaticTerminate();
		ip.clear();
		currentState = GameState::Connect;
	}
	else if (IsKeyPressed(Keys::PERIOD))
	{
		ip.append(".");
	}
	else if (IsKeyPressed(Keys::ZERO))
	{
		ip.append("0");
	}
	else if (IsKeyPressed(Keys::ONE))
	{
		ip.append("1");
	}
	else if (IsKeyPressed(Keys::TWO))
	{
		ip.append("2");
	}
	else if (IsKeyPressed(Keys::THREE))
	{
		ip.append("3");
	}
	else if (IsKeyPressed(Keys::FOUR))
	{
		ip.append("4");
	}
	else if (IsKeyPressed(Keys::FIVE))
	{
		ip.append("5");
	}
	else if (IsKeyPressed(Keys::SIX))
	{
		ip.append("6");
	}
	else if (IsKeyPressed(Keys::SEVEN))
	{
		ip.append("7");
	}
	else if (IsKeyPressed(Keys::EIGHT))
	{
		ip.append("8");
	}
	else if (IsKeyPressed(Keys::NINE))
	{
		ip.append("9");
	}
	else if (IsKeyPressed(Keys::BACKSPACE))
	{
		ip.pop_back();
	}
	else if (IsKeyPressed(Keys::ENTER))
	{
		Client::Get().SetIP(ip);
		if (Client::Get().Startup())
		{
			Client::Get().Set(&currentState);
			currentState = GameState::ClientWait;
		}
	}
}

void GameStateClientWait(float deltaTime)
{
	DrawScreenText("Please Wait for Server to Start", 0.0f, 100.0f, 27.5f, Colors::White);
}

void GameStatePlay(float deltaTime)
{
	std::vector<std::string>* messages;
	std::unique_lock<std::mutex> lock;
	if (isServer)
	{
		lock = { Server::Get().m, std::defer_lock };
		messages = &Server::Get().messages;
	}
	else
	{
		lock = { Client::Get().m, std::defer_lock };
		messages = &Client::Get().messages;
	}

	if (lock.try_lock())
	{
		while (messages->size() > 0)
		{
			std::vector<std::string> splitMessage = StringSplit((*messages)[0], ' ');

			ResolveMessage(splitMessage);

			messages->erase(messages->begin());

			if (currentState == GameState::Lose || currentState == GameState::Win)
			{
				lock.unlock();
				return;
			}
		}
		lock.unlock();
	}

	DrawScore();

	if (goombaTimer > 0.0f)
	{
		goombaTimer -= deltaTime;
	}
	if (blockTimer > 0.0f)
	{
		blockTimer -= deltaTime;
	}

	TileMap::Get().Update(deltaTime);
	if (isPlayer)
	{
		mario.Update(deltaTime);

		Math::Vector2 pos{ mario.GetPosition() };
		AnimationState anim{ mario.GetAnimationState() };
		std::string message{ "M " + std::to_string(pos.x) + " " + std::to_string(pos.y) + " " + std::to_string(mario.GetVelocity().y)
						+ " " + std::to_string(static_cast<int>(anim)) + " " + std::to_string(mario.IsFacingLeft())};

		NetworkingComponent::Send(message, isServer);
	}
	else
	{
		mario.NonPlayerUpdate(deltaTime);
	}

	if (!isPlayer)
	{
		if (IsMousePressed(0) || IsMousePressed(1))
		{
			Math::Vector2 ClickPosition(Math::Vector2(GetMouseScreenX(), GetMouseScreenY()));
			ClickPosition = Camera::Get().ConvertToWorldPosition(ClickPosition);
			Math::Vector2 offset(Math::Vector2((int)ClickPosition.x % 16, (int)ClickPosition.y % 16));

			ClickPosition = {
				offset.x >= 8 ? ClickPosition.x + (16.0f - offset.x) : ClickPosition.x - offset.x,
				offset.y >= 8 ? ClickPosition.y + (16.0f - offset.y) : ClickPosition.y - offset.y
			};

			std::string message = "";

			if (IsMousePressed(0) && goombaTimer <= 0.0f)
			{
				GoombaList.push_back(Goomba());
				GoombaList[GoombaList.size() - 1].Load(ClickPosition, blockTime, 0, goombaSpawnTime);

				goombaTimer = goombaCooldown;

				message = "G " + std::to_string(ClickPosition.x) + " " + std::to_string(ClickPosition.y) + " " + std::to_string(GetID());
			}
			else if (IsMousePressed(1) && blockTimer <= 0.0f)
			{
				TileMap::Get().AddBlock(ClickPosition, goombaTime, 0);

				blockTimer = blockCooldown;

				message = "B " + std::to_string(ClickPosition.x) + " " + std::to_string(ClickPosition.y);
			}

			if (!message.empty())
			{
				NetworkingComponent::Send(message, isServer);
			}
		}
	}

	ThatOneKoopa.Update(deltaTime);

	for (unsigned int i = 0; i < GoombaList.size(); i++)
	{
		GoombaList[i].Update(deltaTime);
		if (GoombaList[i].TimeUp())
		{
			GoombaList[i].Unload();
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

	if (mario.GetPosition().x >= 3184.0f && isPlayer)
	{
		currentState = GameState::Win;
	}
	if (mario.GetPosition().y > GetScreenHeight() && isPlayer)
	{
		currentState = GameState::Lose;
	}

	iFrames -= deltaTime;
	if (iFrames < 0.0f)
	{
		iFrames = 0.0f;
	}
}


void GameStateLose(float deltaTime) 
{
	if (isPlayer)
	{
		NetworkingComponent::Send("L", isServer);
	}
	if (isServer)
	{
		isPlayer = Server::Get().GetNewPlayer();
	}
	Reset();
	currentState = GameState::Play;
}

void GameStateWin(float deltaTime) 
{
	if (isPlayer)
	{
		NetworkingComponent::Send("W", isServer);
		if (isServer)
		{
			scores[0] += scores.size() - 1;
		}
		else
		{
			scores[Client::Get().GetClientID() + 1] += scores.size() - 1;
		}
		SendScoreMessage();
	}
	if (isServer)
	{
		isPlayer = Server::Get().GetNewPlayer();
		currentPlayer = Server::Get().GetCurrentPlayer();
	}
	Reset();
	currentState = GameState::Play;
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
	case GameState::ClientStart:
		GameStateClientStart(deltaTime);
		break;
	case GameState::ClientWait:
		GameStateClientWait(deltaTime);
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
	if (currentState != GameState::Connect && currentState != GameState::Start)
	{
		if (isServer)
		{
			Server::Get().StaticTerminate();
		}
		else
		{
			Client::Get().StaticTerminate();
		}
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