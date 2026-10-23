#include "cart.h"
#include "flush.h"
#include "persist.h"
#include "sound.h"
#include "host.h"
#include "input.h"
#include "score.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <stdio.h>

#define GRID 16
#define LED 4
#define CANVAS_W 128
#define CANVAS_H 192
#define FIELD_X ((CANVAS_W - GRID * LED) / 2)
#define CTRL_H 64
#define CTRL_Y (CANVAS_H - CTRL_H)
#define FIELD_Y ((CTRL_Y - GRID * LED) / 2)
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
        snprintf(path, sizeof path, "tools/sim/img/%s", file);
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
    int ctrl_rest = CTRL_Y + (CTRL_H - 32) / 2;
    SDL_Rect stick = {
        MARGIN + sx * travel,
        ctrl_rest + sy * travel,
        32,
        32,
    };
    SDL_Rect button = {
        64 + MARGIN,
        ctrl_rest,
        32,
        32,
    };

    SDL_SetRenderDrawColor(renderer, BG, BG, BG, 255);
    SDL_RenderClear(renderer);

    for (y = 0; y < GRID; y++) {
        for (x = 0; x < GRID; x++) {
            SDL_Rect cell = {FIELD_X + x * LED, FIELD_Y + y * LED, LED, LED};

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

/* The canvas is 1:1 with the art. One integer scale of that whole picture
 * keeps every source pixel the same size. Per-sprite scaling does not. */
static void present_canvas(SDL_Renderer *renderer, SDL_Texture *canvas)
{
    int out_w;
    int out_h;
    int scale;
    int fit_h;
    SDL_Rect dst;

    SDL_SetRenderTarget(renderer, NULL);
    SDL_GetRendererOutputSize(renderer, &out_w, &out_h);
    scale = out_w / CANVAS_W;
    fit_h = out_h / CANVAS_H;
    if (fit_h < scale)
        scale = fit_h;
    if (scale < 1)
        scale = 1;
    dst.x = (out_w - CANVAS_W * scale) / 2;
    dst.y = (out_h - CANVAS_H * scale) / 2;
    dst.w = CANVAS_W * scale;
    dst.h = CANVAS_H * scale;
    SDL_SetRenderDrawColor(renderer, BG, BG, BG, 255);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, canvas, NULL, &dst);
    SDL_RenderPresent(renderer);
}

static TTF_Font *open_font(void)
{
    const char *rels[] = {
        "tools/assets/fonts/proggy-tiny.ttf",
        "../assets/fonts/proggy-tiny.ttf",
        NULL,
    };
    char *base = SDL_GetBasePath();
    char path[1024];
    int i;
    TTF_Font *font = NULL;

    if (base != NULL) {
        snprintf(path, sizeof path, "%s../assets/fonts/proggy-tiny.ttf", base);
        SDL_free(base);
        font = TTF_OpenFont(path, 16);
    }
    for (i = 0; font == NULL && rels[i] != NULL; i++)
        font = TTF_OpenFont(rels[i], 16);
    if (font == NULL) {
        SDL_Log("TTF_OpenFont failed: %s", TTF_GetError());
        return NULL;
    }
    TTF_SetFontHinting(font, TTF_HINTING_MONO);
    TTF_SetFontKerning(font, 0);
    return font;
}

static SDL_Texture *fps_text(SDL_Renderer *renderer, TTF_Font *font, int fps,
                             int *w, int *h)
{
    char buf[16];
    SDL_Color color = {204, 204, 204, 255};
    SDL_Surface *surface;
    SDL_Texture *tex;

    snprintf(buf, sizeof buf, "%d FPS", fps);
    surface = TTF_RenderText_Solid(font, buf, color);
    if (surface == NULL)
        return NULL;
    tex = SDL_CreateTextureFromSurface(renderer, surface);
    if (tex != NULL) {
        *w = surface->w;
        *h = surface->h;
    }
    SDL_FreeSurface(surface);
    return tex;
}

/* Sleep the rest of a 1/60 s frame, then spin the last slice. SDL_Delay
 * wakes late, and that late wake is the uneven ship step. */
static void pace(Uint64 frame_start, Uint64 freq)
{
    const double target = 1.0 / 60.0;

    for (;;) {
        Uint64 now = SDL_GetPerformanceCounter();
        double left = target - (double)(now - frame_start) / (double)freq;

        if (left <= 0.0)
            return;
        if (left > 0.002) {
            Uint32 ms = (Uint32)((left - 0.001) * 1000.0);

            if (ms > 0)
                SDL_Delay(ms);
        } else {
            break;
        }
    }
    for (;;) {
        Uint64 now = SDL_GetPerformanceCounter();

        if ((double)(now - frame_start) / (double)freq >= target)
            return;
    }
}

static int open_cart(void)
{
    char *base;
    char path[1024];
    int rc;

    rc = cart_read_file("cart_placed.json");
    if (rc >= 0)
        return rc;
    base = SDL_GetBasePath();
    if (base == NULL)
        return 0;
    snprintf(path, sizeof path, "%s../../cart_placed.json", base);
    SDL_free(base);
    rc = cart_read_file(path);
    return rc > 0;
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
    int loaded;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
        SDL_Log("IMG_Init failed: %s", IMG_GetError());
        SDL_Quit();
        return 1;
    }
    if (TTF_Init() != 0) {
        SDL_Log("TTF_Init failed: %s", TTF_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    /* The drivers latch the first picture before a window exists, so the
     * open does not flash an empty frame. An empty socket leaves them
     * in reset, which is a dark field. */
    loaded = open_cart();
    board_init(&board);
    sim_bind(&board);
    sound_bind(&board);
    input_reset();
    if (loaded) {
        persist_bind(cart_eeprom());
        score_load();
        cart_reset();
        drivers_init();
        drivers_flush();
        board_advance(&board, MAX7219_WAKE_NS);
        if (board_fault(&board) != 0) {
            SDL_Log("display bring-up saw a timing fault");
            TTF_Quit();
            IMG_Quit();
            SDL_Quit();
            return 1;
        }
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    window = SDL_CreateWindow("Maki Nita simulator",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              CANVAS_W * SCALE,
                              CANVAS_H * SCALE,
                              SDL_WINDOW_HIDDEN);
    if (window == NULL) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1,
                                 SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    if (renderer == NULL)
        renderer = SDL_CreateRenderer(window, -1,
                                     SDL_RENDERER_SOFTWARE | SDL_RENDERER_TARGETTEXTURE);
    if (renderer == NULL) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
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
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Texture *canvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                            SDL_TEXTUREACCESS_TARGET,
                                            CANVAS_W, CANVAS_H);
    if (canvas == NULL) {
        SDL_Log("canvas texture failed: %s", SDL_GetError());
        SDL_DestroyTexture(led_on);
        SDL_DestroyTexture(led_off);
        SDL_DestroyTexture(stick_tex);
        SDL_DestroyTexture(button_tex);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    SDL_SetTextureScaleMode(canvas, SDL_ScaleModeNearest);
    SDL_SetTextureScaleMode(led_on, SDL_ScaleModeNearest);
    SDL_SetTextureScaleMode(led_off, SDL_ScaleModeNearest);

    TTF_Font *font = open_font();
    if (font == NULL) {
        SDL_DestroyTexture(canvas);
        SDL_DestroyTexture(led_on);
        SDL_DestroyTexture(led_off);
        SDL_DestroyTexture(stick_tex);
        SDL_DestroyTexture(button_tex);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderTarget(renderer, canvas);
    draw(renderer, led_on, led_off, stick_tex, button_tex, &board, 0, 0, 0);
    present_canvas(renderer, canvas);
    SDL_ShowWindow(window);
    present_canvas(renderer, canvas);

    Uint64 freq = SDL_GetPerformanceFrequency();
    Uint64 frame_start = 0;
    SDL_Texture *fps_tex = NULL;
    int fps_show = -1;
    int fps_w = 0;
    int fps_h = 0;

    sound_open();
    sound_render();

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
        Uint64 now = SDL_GetPerformanceCounter();

        if (frame_start != 0 && now > frame_start) {
            double dt = (double)(now - frame_start) / (double)freq;
            int fps = (int)(1.0 / dt + 0.5);

            if (fps > 999)
                fps = 999;
            if (fps != fps_show) {
                SDL_Texture *next = fps_text(renderer, font, fps, &fps_w, &fps_h);

                if (next != NULL) {
                    SDL_SetTextureScaleMode(next, SDL_ScaleModeNearest);
                    SDL_DestroyTexture(fps_tex);
                    fps_tex = next;
                    fps_show = fps;
                }
            }
        }
        frame_start = now;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = 0;
            if (event.type == SDL_KEYDOWN &&
                event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                running = 0;
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
        board_set_switches(&board, up, down, left, right, pressed);
        input_tick(board_sense(&board, AVR_PD1) == SIG_L,
                   board_sense(&board, AVR_PD2) == SIG_L,
                   board_sense(&board, AVR_PD3) == SIG_L,
                   board_sense(&board, AVR_PD4) == SIG_L,
                   board_sense(&board, AVR_PD5) == SIG_L,
                   &in);
        if (loaded) {
            cart_tick(&in);
            drivers_flush();
        }
        board_advance(&board, 16000000);

        SDL_SetRenderTarget(renderer, canvas);
        draw(renderer, led_on, led_off, stick_tex, button_tex, &board,
             sx, sy, pressed);
        if (fps_tex != NULL) {
            SDL_Rect dst = {
                CANVAS_W - fps_w - 2,
                CANVAS_H - fps_h - 2,
                fps_w,
                fps_h,
            };

            SDL_RenderCopy(renderer, fps_tex, NULL, &dst);
        }
        present_canvas(renderer, canvas);
        sound_render();
        pace(frame_start, freq);
    }

    sound_close();

    SDL_DestroyTexture(fps_tex);
    SDL_DestroyTexture(canvas);
    SDL_DestroyTexture(led_on);
    SDL_DestroyTexture(led_off);
    SDL_DestroyTexture(stick_tex);
    SDL_DestroyTexture(button_tex);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
