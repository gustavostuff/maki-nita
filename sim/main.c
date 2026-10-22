#include "flush.h"
#include "game.h"
#include "host.h"
#include "input.h"

#include <SDL.h>
#include <SDL_image.h>

#include <stdio.h>

#define GRID 16
#define LED 8
#define CANVAS_W (GRID * LED)
#define CANVAS_H 192
#define SCALE 4
#define MARGIN 16
#define BG ((Uint8)(0.0 * 255)) // fully dark

static SDL_Texture *load_tex(SDL_Renderer *renderer, const char *file)
{
    char *base = SDL_GetBasePath();
    char path[1024];
    SDL_Surface *surface;
    SDL_Texture *tex;

    if (base != NULL) {
        snprintf(path, sizeof path, "%simg/%s", base, file);
        SDL_free(base);
        surface = IMG_Load(path);
    } else {
        surface = NULL;
    }
    if (surface == NULL) {
        snprintf(path, sizeof path, "sim/img/%s", file);
        surface = IMG_Load(path);
    }
    if (surface == NULL) {
        snprintf(path, sizeof path, "img/%s", file);
        surface = IMG_Load(path);
    }
    if (surface == NULL) {
        SDL_Log("IMG_Load %s failed: %s", file, IMG_GetError());
        return NULL;
    }
    tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (tex == NULL)
        SDL_Log("texture %s failed: %s", file, SDL_GetError());
    return tex;
}

static void draw(SDL_Renderer *renderer, SDL_Texture *led_on, SDL_Texture *led_off,
                 SDL_Texture *stick_tex, SDL_Texture *button_tex,
                 const Board *board, int sx, int sy, int pressed)
{
    int x;
    int y;
    int travel = (sx != 0 && sy != 0) ? 6 : 8;
    SDL_Rect stick = {
        MARGIN + sx * travel,
        GRID * LED + MARGIN + sy * travel,
        32,
        32,
    };
    SDL_Rect button = {
        64 + MARGIN,
        GRID * LED + MARGIN,
        32,
        32,
    };

    SDL_SetRenderDrawColor(renderer, BG, BG, BG, 255);
    SDL_RenderClear(renderer);

    for (y = 0; y < GRID; y++) {
        for (x = 0; x < GRID; x++) {
            SDL_Rect cell = {x * LED, y * LED, LED, LED};

            SDL_RenderCopy(renderer,
                           board_led_on(board, x, y) ? led_on : led_off,
                           NULL, &cell);
        }
    }

    SDL_RenderCopy(renderer, stick_tex, NULL, &stick);
    if (pressed)
        SDL_SetTextureColorMod(button_tex, 150, 150, 150);
    else
        SDL_SetTextureColorMod(button_tex, 255, 255, 255);
    SDL_RenderCopy(renderer, button_tex, NULL, &button);
}

/* The button-up event is lost when the window moves under the pointer.
 * The platform button mask is what ends the drag. */
static void drag_update(SDL_Window *window, int *dragging, int button,
                        int anchor_mx, int anchor_my, int anchor_wx, int anchor_wy)
{
    int mx;
    int my;
    Uint32 held;

    if (!*dragging)
        return;
    held = SDL_GetGlobalMouseState(&mx, &my);
    if ((held & SDL_BUTTON(button)) == 0) {
        *dragging = 0;
        SDL_CaptureMouse(SDL_FALSE);
        return;
    }
    SDL_SetWindowPosition(window,
                          anchor_wx + mx - anchor_mx,
                          anchor_wy + my - anchor_my);
}

int main(void)
{
    static Board board;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *led_on;
    SDL_Texture *led_off;
    SDL_Texture *stick_tex;
    SDL_Texture *button_tex;
    int running = 1;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
        SDL_Log("IMG_Init failed: %s", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    /* The drivers latch the first picture before a window exists, so the
     * open does not flash an empty frame. */
    board_init(&board);
    sim_bind(&board);
    input_reset();
    game_reset();
    drivers_init();
    drivers_flush();
    board_advance(&board, MAX7219_WAKE_NS);
    if (board_fault(&board) != 0) {
        SDL_Log("display bring-up saw a timing fault");
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    window = SDL_CreateWindow("RetroDiode",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              CANVAS_W * SCALE,
                              CANVAS_H * SCALE,
                              SDL_WINDOW_HIDDEN | SDL_WINDOW_BORDERLESS);
    if (window == NULL) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == NULL) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    SDL_RenderSetLogicalSize(renderer, CANVAS_W, CANVAS_H);
    SDL_RenderSetIntegerScale(renderer, SDL_TRUE);

    led_on = load_tex(renderer, "led_on.png");
    led_off = load_tex(renderer, "led_off.png");
    stick_tex = load_tex(renderer, "stick.png");
    button_tex = load_tex(renderer, "button.png");
    if (led_on == NULL || led_off == NULL || stick_tex == NULL || button_tex == NULL) {
        SDL_DestroyTexture(led_on);
        SDL_DestroyTexture(led_off);
        SDL_DestroyTexture(stick_tex);
        SDL_DestroyTexture(button_tex);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    int dragging = 0;
    int drag_button = 0;
    int drag_mx = 0;
    int drag_my = 0;
    int drag_wx = 0;
    int drag_wy = 0;
    int hold_sx = 0;
    int hold_sy = 0;
    int hold_pressed = 0;

    draw(renderer, led_on, led_off, stick_tex, button_tex, &board, 0, 0, 0);
    SDL_RenderPresent(renderer);
    SDL_ShowWindow(window);
    SDL_RenderPresent(renderer);

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
            if (event.type == SDL_KEYDOWN &&
                event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                running = 0;
            if (event.type == SDL_MOUSEBUTTONDOWN && !dragging &&
                (event.button.button == SDL_BUTTON_LEFT ||
                 event.button.button == SDL_BUTTON_RIGHT)) {
                dragging = 1;
                drag_button = event.button.button;
                SDL_GetGlobalMouseState(&drag_mx, &drag_my);
                SDL_GetWindowPosition(window, &drag_wx, &drag_wy);
                SDL_CaptureMouse(SDL_TRUE);
            }
            if (event.type == SDL_MOUSEBUTTONUP && dragging &&
                event.button.button == drag_button) {
                dragging = 0;
                SDL_CaptureMouse(SDL_FALSE);
            }
            if (event.type == SDL_KEYDOWN &&
                (event.key.keysym.mod & KMOD_CTRL) &&
                !event.key.repeat) {
                int scale = 0;

                if (event.key.keysym.scancode == SDL_SCANCODE_1)
                    scale = 1;
                if (event.key.keysym.scancode == SDL_SCANCODE_2)
                    scale = 2;
                if (event.key.keysym.scancode == SDL_SCANCODE_3)
                    scale = 3;
                if (event.key.keysym.scancode == SDL_SCANCODE_4)
                    scale = 4;
                if (scale != 0)
                    SDL_SetWindowSize(window, CANVAS_W * scale, CANVAS_H * scale);
            }
        }
        drag_update(window, &dragging, drag_button,
                    drag_mx, drag_my, drag_wx, drag_wy);

        if (!dragging) {
            keys = SDL_GetKeyboardState(NULL);
            up = (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) ? 1 : 0;
            down = (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) ? 1 : 0;
            left = (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) ? 1 : 0;
            right = (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) ? 1 : 0;
            sx = 0;
            sy = 0;
            if (up && !down)
                sy = -1;
            if (down && !up)
                sy = 1;
            if (left && !right)
                sx = -1;
            if (right && !left)
                sx = 1;
            pressed = keys[SDL_SCANCODE_SPACE] ? 1 : 0;
            hold_sx = sx;
            hold_sy = sy;
            hold_pressed = pressed;
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
        }

        draw(renderer, led_on, led_off, stick_tex, button_tex, &board,
             hold_sx, hold_sy, hold_pressed);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(led_on);
    SDL_DestroyTexture(led_off);
    SDL_DestroyTexture(stick_tex);
    SDL_DestroyTexture(button_tex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
