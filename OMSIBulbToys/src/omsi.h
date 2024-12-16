#pragma once
#include <string>

#include "../core/bulbtoys.h"

inline class Game
{
	static inline Game* first = nullptr;
	Game* next = nullptr;
public:
	Game()
	{
		if (!Game::first)
		{
			Game::first = this;
		}
		else
		{
			auto game = Game::first;
			while (game->next)
			{
				game = game->next;
			}
			game->next = this;
		}
	}

	static const char* Init()
	{
		auto game = Game::first;
		while (game)
		{
			auto version = game->Check();
			if (version)
			{
				OMSI = game;
				return version;
			}
			game = game->next;
		}
		return nullptr;
	}

	virtual const char* Check() = 0;

	virtual LPVOID BulbToys_GetD3DDevice9() = 0;
	virtual uintptr_t BulbToys_GetDirectInputCl() = 0;

} *OMSI;