#pragma once

enum class GameState
{
	Start,
	Connect,
	ServerStart,
	ClientStart,
	ClientWait,
	Play,
	Win,
	Lose
};