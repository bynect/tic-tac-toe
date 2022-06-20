#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include <SDL2/SDL.h>

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

#define FLASH_COUNT 8
#define FLASH_DELAY 60

#define COLOR_BACKGROUND 18, 18, 18, 255
#define COLOR_FOREGROUND 255, 255, 255, 255
#define COLOR_EMPTY 48, 48, 48, 255
#define COLOR_NOUGHT 205, 75, 0, 255
#define COLOR_CROSS 0, 162, 237, 255

typedef struct {
	int x, y;
} vec2i;

double rad(double angle)
{
	const double PI = 3.14159265;
	return angle * PI / 180;
}

// Angle in degrees
vec2i rotate(vec2i pos, vec2i orig, double angle)
{
	double radians = rad(angle);

	int px = pos.x - orig.x;
	int py = pos.y - orig.y;

	vec2i new = {
		.x = px*SDL_cos(radians) - py*SDL_sin(radians),
		.y = px*SDL_sin(radians) + py*SDL_cos(radians),
	};

	new.x += orig.x;
	new.y += orig.y;

	return new;
}

int distance(vec2i p1, vec2i p2)
{
	return SDL_sqrt(SDL_pow(p1.x - p2.x, 2) + SDL_pow(p1.y - p2.y, 2));
}

typedef enum {
	EMPTY,
	NOUGHT,
	CROSS,
} Cell;

Cell cells[3][3];
bool selected[3][3];
Cell current = CROSS;

const char cell_char[3] = {'_', 'o', 'x'};
SDL_Texture *texture;

void render_circle(SDL_Renderer *renderer, int x, int y, int r1, int r2)
{
    for (int w = 0; w < r1 * 2; w++)
    {
        for (int h = 0; h < r1 * 2; h++)
        {
            int dx = r1 - w;
            int dy = r1 - h;
			int t = dx * dx + dy * dy;
            if (t <= r1 * r1 && t > r2 * r2) SDL_RenderDrawPoint(renderer, x + dx, y + dy);
        }
    }
}

// < height, angle, bounding size
// > width, coords
void render_cross(SDL_Renderer *renderer, SDL_Rect bounding_rect, int height)
{
	int width = bounding_rect.w * SDL_sqrt(2) - height;

	int base_x = bounding_rect.x + bounding_rect.w / 2 - width / 2;
	int base_y = bounding_rect.y + bounding_rect.h / 2 - height / 2;

	SDL_Rect rect = {
		.x = base_x,
		.y = base_y,
		.w = width,
		.h = height,
	};

	cz(SDL_RenderCopyEx(renderer, texture, NULL, &rect, 45, NULL, SDL_FLIP_NONE));
	cz(SDL_RenderCopyEx(renderer, texture, NULL, &rect, -45, NULL, SDL_FLIP_NONE));
}

void render_grid(SDL_Renderer *renderer, bool select)
{
	cz(SDL_SetRenderDrawColor(renderer, COLOR_BACKGROUND));
	cz(SDL_RenderClear(renderer));

	SDL_Rect rect;
	rect.w = rect.h = CELL_SIZE;
	rect.y = CELL_SPACE;

	debug("cells:\n");

	int flash = 0;
	do
	{
		rect.y = CELL_SPACE;

		for (int i = 0; i < 3; ++i)
		{
			rect.x = CELL_SPACE;
			for (int j = 0; j < 3; ++j)
			{
				if (select && selected[i][j] && flash % 2 == 0)
				{
					SDL_SetRenderDrawColor(renderer, COLOR_FOREGROUND);
					SDL_RenderFillRect(renderer, &rect);
				}
				else
				{
					cz(SDL_SetRenderDrawColor(renderer, COLOR_EMPTY));
					cz(SDL_RenderFillRect(renderer, &rect));

					cz(SDL_SetRenderDrawColor(renderer, COLOR_FOREGROUND));
					if (cells[i][j] == NOUGHT) render_circle(renderer, rect.x + CELL_SIZE / 2, rect.y + CELL_SIZE / 2, CELL_SIZE / 2, CELL_SIZE / 3);
					else if (cells[i][j] == CROSS) render_cross(renderer, rect, CELL_SIZE / 4);
				}

				debug("%c ", cell_char[cells[i][j]]);
				rect.x += CELL_SIZE + CELL_SPACE;
			}
			debug("\n");
			rect.y += CELL_SIZE + CELL_SPACE;
		}

		if (select)
		{
			SDL_RenderPresent(renderer);
			SDL_Delay(FLASH_DELAY);
		}
		else break;

		++flash;
		debug("flash=%d\n", flash);
	}
	while (flash < FLASH_COUNT);

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
					debug("Alredy set to %c\n", cell_char[cells[i][j]]);
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
			for (int j = 0; j < 3; ++j) selected[i][j] = true;
			return cells[i][0];
		}

		if (cells[0][i] == cells[1][i] && cells[1][i] == cells[2][i] && cells[0][i] != EMPTY)
		{
			for (int j = 0; j < 3; ++j) selected[i][j] = true;
			return cells[0][i];
		}
	}

	if (cells[1][1] != EMPTY)
	{
		if (cells[0][0] == cells[1][1] && cells[1][1] == cells[2][2])
		{
			selected[0][0] = true;
			selected[1][1] = true;
			selected[2][2] = true;
			return cells[1][1];
		}

		if (cells[0][2] == cells[1][1] && cells[1][1] == cells[2][0])
		{
			selected[0][2] = true;
			selected[1][1] = true;
			selected[2][0] = true;
			return cells[1][1];
		}
	}

	return EMPTY;
}

void reset()
{
	memset(cells, 0, sizeof(cells));
	memset(selected, 0, sizeof(selected));
	current = CROSS;
}

bool full()
{
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			if (cells[i][j] == EMPTY) return false;
		}
	}

	return true;
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

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_UNKNOWN, SDL_TEXTUREACCESS_TARGET, 128, 128);
	cn(texture);

	cz(SDL_SetRenderTarget(renderer, texture));

	cz(SDL_SetRenderDrawColor(renderer, COLOR_FOREGROUND));
	cz(SDL_RenderClear(renderer));

	cz(SDL_SetRenderTarget(renderer, NULL));

	bool running = true;
	bool changed = true;

	while (running)
	{
		if (changed)
		{
			Cell winner = win();
			render_grid(renderer, false);

			if (winner != EMPTY)
			{
				render_grid(renderer, true);
				debug("%c won\n", cell_char[winner]);
				reset();
			}
			else if (full()) reset();
			else changed = false;
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
