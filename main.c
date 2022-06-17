#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#ifdef DEBUG
#define debug(...) printf(__VA_ARGS__)
#else
#define debug(...)
#endif

#define error() do { printf("%s:%s:%d ERROR: %s\n", __FILE__, __func__, __LINE__, SDL_GetError()); abort(); } while (false)
#define cn(...) if ((__VA_ARGS__) == NULL) error();
#define cz(...) if ((__VA_ARGS__) < 0) error();

#define SCREEN_WIDTH 800.0
#define SCREEN_HEIGHT 800.0

#define CELL_SIZE (SCREEN_HEIGHT / 4)
#define CELL_SPACE (CELL_SIZE / 4)

#define COLOR_EMPTY 48, 48, 48, 255
#define COLOR_NOUGHT 205, 75, 0, 255
#define COLOR_CROSS 0, 162, 237, 255

typedef enum {
	EMPTY,
	NOUGHT,
	CROSS,
} Cell;

Cell cells[3][3];
Cell current = CROSS;
const char cell_char[3] = {'_', 'o', 'x'};

void render(SDL_Renderer *renderer)
{
	cz(SDL_SetRenderDrawColor(renderer, 18, 18, 18, 255));
	cz(SDL_RenderClear(renderer));

	SDL_Rect rect;
	rect.w = rect.h = CELL_SIZE;
	rect.y = CELL_SPACE;

	debug("cells:\n");
	for (int i = 0; i < 3; ++i)
	{
		rect.x = CELL_SPACE;
		for (int j = 0; j < 3; ++j)
		{
			switch (cells[i][j])
			{
				case EMPTY:
					SDL_SetRenderDrawColor(renderer, COLOR_EMPTY);
					SDL_RenderFillRect(renderer, &rect);
					break;

				case NOUGHT:
					SDL_SetRenderDrawColor(renderer, COLOR_NOUGHT);
					SDL_RenderFillRect(renderer, &rect);
					break;

				case CROSS:
					SDL_SetRenderDrawColor(renderer, COLOR_CROSS);
					SDL_RenderFillRect(renderer, &rect);
					break;
			}

			debug("%c ", cell_char[cells[i][j]]);
			rect.x += CELL_SIZE + CELL_SPACE;
		}

		debug("\n");
		rect.y += CELL_SIZE + CELL_SPACE;
	}

	SDL_RenderPresent(renderer);
}

bool click(SDL_MouseButtonEvent *button)
{
	if (button->button != SDL_BUTTON_LEFT) return false;

	int click_x = button->x, click_y = button->y;
	debug("left-click: x=%d y=%d\n", click_x, click_y);

	int cell_x, cell_y = CELL_SPACE;
	for (int i = 0; i < 3; ++i)
	{
		cell_x = CELL_SPACE;
		for (int j = 0; j < 3; ++j)
		{
			if (click_x >= cell_x && click_x <= cell_x + CELL_SIZE &&
				click_y >= cell_y && click_y <= cell_y + CELL_SIZE)
			{
				if (cells[i][j] == EMPTY)
				{
					cells[i][j] = current;
					current = current == NOUGHT ? CROSS : NOUGHT;
				}
				else
				{
					printf("Alredy set to %c\n", cell_char[cells[i][j]]);
				}
				return true;
			}
			cell_x += CELL_SIZE + CELL_SPACE;
		}
		cell_y += CELL_SIZE + CELL_SPACE;
	}
	return false;
}

Cell win()
{
	for (int i = 0; i < 3; ++i)
	{
		if (cells[i][0] == cells[i][1] && cells[i][1] == cells[i][2] && cells[i][0] != EMPTY)
		{
			return cells[i][0];
		}

		if (cells[0][i] == cells[1][i] && cells[1][i] == cells[2][i] && cells[0][i] != EMPTY)
		{
			return cells[0][i];
		}
	}

	return (cells[1][1] != EMPTY
		&& ((cells[0][0] == cells[1][1] && cells[1][1] == cells[2][2])
		|| (cells[0][2] == cells[1][1] && cells[1][1] == cells[2][0])))
		? cells[1][1] : EMPTY;
}

void reset()
{
	memset(cells, 0, sizeof(cells));
	current = NOUGHT;
}

void full()
{
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			if (cells[i][j] == EMPTY) return;
		}
	}

	reset();
}

int main()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0)
	{
		printf("Unable to initialize SDL2: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Window *window = SDL_CreateWindow("tic-tac-toe", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
										  SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	cn(window);

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	cn(renderer);

	bool running = true;
	bool changed = true;

	while (running)
	{
		if (changed)
		{
			Cell winner = win();
			if (winner != EMPTY)
			{
				printf("%c won\n", cell_char[winner]);
				reset();
			}
			else full();

			render(renderer);
			changed = false;
		}

		SDL_Event event;
		SDL_WaitEvent(&event);
		if (event.type == SDL_QUIT) running = false;
		else if (event.type == SDL_MOUSEBUTTONUP) changed = click(&event.button);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

