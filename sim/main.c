#include "flush.h"
#include "game.h"
#include "host.h"
#include "input.h"

#include <SDL.h>

#define GRID 16

static void fill_circle(SDL_Renderer *renderer, int cx, int cy, int radius)
{
    int r2 = radius * radius;
    int dy;

    for (dy = -radius; dy <= radius; dy++) {
        int dx = 0;

        while ((dx + 1) * (dx + 1) + dy * dy <= r2)
            dx++;
        SDL_RenderDrawLine(renderer, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

static void draw_arrow(SDL_Renderer *renderer, int x0, int y0, int x1, int y1)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int bx = x1 - dx / 3;
    int by = y1 - dy / 3;
    int px = -dy / 3;
    int py = dx / 3;

    if (px == 0 && py == 0) {
        px = (dy == 0) ? 0 : (dy > 0 ? -4 : 4);
        py = (dx == 0) ? 0 : (dx > 0 ? 4 : -4);
    }

    SDL_RenderDrawLine(renderer, x0, y0, x1, y1);
    SDL_RenderDrawLine(renderer, x0 + (py > 0) - (py < 0), y0 + (px > 0) - (px < 0),
                       x1 + (py > 0) - (py < 0), y1 + (px > 0) - (px < 0));
    SDL_RenderDrawLine(renderer, x1, y1, bx + px, by + py);
    SDL_RenderDrawLine(renderer, x1, y1, bx - px, by - py);
}

static void draw(SDL_Renderer *renderer, const Board *board, int sx, int sy, int pressed)
{
    const int win_w = 420;
    const int pitch = 20;
    const int led_r = 7;
    const int grid_span = (GRID - 1) * pitch;
    const int origin_x = (win_w - grid_span) / 2;
    const int origin_y = 48;
    int x;
    int y;

    SDL_SetRenderDrawColor(renderer, 22, 24, 28, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 12, 12, 14, 255);
    {
        SDL_Rect well = {
            origin_x - 18,
            origin_y - 18,
            grid_span + 36,
            grid_span + 36,
        };
        SDL_RenderFillRect(renderer, &well);
    }

    for (y = 0; y < GRID; y++) {
        for (x = 0; x < GRID; x++) {
            if (board_led_on(board, x, y))
                SDL_SetRenderDrawColor(renderer, 230, 28, 28, 255);
            else
                SDL_SetRenderDrawColor(renderer, 52, 10, 10, 255);
            fill_circle(renderer,
                        origin_x + x * pitch,
                        origin_y + y * pitch,
                        led_r);
        }
    }

    /* Stick, seen from above. sx and sy are -1, 0, or 1. */
    {
        const int cx = 118;
        const int cy = 530;
        const int dirs[8][2] = {
            {0, -1}, {1, -1}, {1, 0}, {1, 1},
            {0, 1}, {-1, 1}, {-1, 0}, {-1, -1},
        };
        int i;
        int travel = (sx != 0 && sy != 0) ? 10 : 16;

        SDL_SetRenderDrawColor(renderer, 40, 42, 48, 255);
        fill_circle(renderer, cx, cy, 50);
        SDL_SetRenderDrawColor(renderer, 24, 26, 30, 255);
        fill_circle(renderer, cx, cy, 40);

        for (i = 0; i < 8; i++) {
            int dx = dirs[i][0];
            int dy = dirs[i][1];
            int ring = (dx != 0 && dy != 0) ? 41 : 58;
            int step = (dx != 0 && dy != 0) ? 10 : 14;
            int hot = (sx == dx && sy == dy && (sx != 0 || sy != 0));
            int x0 = cx + dx * ring;
            int y0 = cy + dy * ring;
            int x1 = x0 + dx * step;
            int y1 = y0 + dy * step;

            if (hot)
                SDL_SetRenderDrawColor(renderer, 235, 64, 48, 255);
            else
                SDL_SetRenderDrawColor(renderer, 96, 98, 104, 255);
            draw_arrow(renderer, x0, y0, x1, y1);
        }

        if (sx != 0 || sy != 0)
            SDL_SetRenderDrawColor(renderer, 240, 52, 44, 255);
        else
            SDL_SetRenderDrawColor(renderer, 186, 34, 34, 255);
        fill_circle(renderer, cx + sx * travel, cy + sy * travel, 16);
    }

    /* Action button. Space holds it down. */
    {
        int by = pressed ? 534 : 530;
        int cap = pressed ? 36 : 40;

        SDL_SetRenderDrawColor(renderer, 36, 36, 40, 255);
        fill_circle(renderer, 300, 530, 48);
        if (pressed)
            SDL_SetRenderDrawColor(renderer, 150, 18, 18, 255);
        else
            SDL_SetRenderDrawColor(renderer, 196, 26, 26, 255);
        fill_circle(renderer, 300, by, cap);
    }
}

int main(void)
{
    static Board board;
    SDL_Window *window;
    SDL_Renderer *renderer;
    int running = 1;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("RetroDiode",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              420, 680,
                              SDL_WINDOW_SHOWN);
    if (window == NULL) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == NULL) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    board_init(&board);
    sim_bind(&board);
    input_reset();
    game_reset();
    drivers_init();
    drivers_flush();
    board_advance(&board, MAX7219_WAKE_NS);
    if (board_fault(&board) != 0) {
        SDL_Log("display bring-up saw a timing fault");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    while (running) {
        SDL_Event event;
        const Uint8 *keys;
        int up;
        int down;
        int left;
        int right;
        Input in;
        int sx = 0;
        int sy = 0;
        int pressed;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = 0;
        }

        keys = SDL_GetKeyboardState(NULL);
        up = (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) ? 1 : 0;
        down = (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) ? 1 : 0;
        left = (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) ? 1 : 0;
        right = (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) ? 1 : 0;
        if (up && !down)
            sy = -1;
        if (down && !up)
            sy = 1;
        if (left && !right)
            sx = -1;
        if (right && !left)
            sx = 1;
        pressed = keys[SDL_SCANCODE_SPACE] ? 1 : 0;
        board_set_switches(&board, up, down, left, right, pressed);
        input_tick(board_sense(&board, AVR_PD1) == SIG_L,
                   board_sense(&board, AVR_PD2) == SIG_L,
                   board_sense(&board, AVR_PD3) == SIG_L,
                   board_sense(&board, AVR_PD4) == SIG_L,
                   board_sense(&board, AVR_PD5) == SIG_L,
                   &in);
        game_tick(&in);
        drivers_flush();
        board_advance(&board, 16000000);

        draw(renderer, &board, sx, sy, pressed);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
