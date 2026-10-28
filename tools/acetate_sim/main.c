#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GRID 16
#define LED 4
#define FIELD_SIZE (GRID * LED)

#define CANVAS_W 96
#define CANVAS_H 104
#define FIELD_X ((CANVAS_W - FIELD_SIZE) / 2)
#define FIELD_Y 24

#define NUM_COLORS 8
#define SWATCH_W 8
#define SWATCH_H 8
#define SWATCH_GAP 2
#define SWATCH_STRIPE_W (NUM_COLORS * SWATCH_W + (NUM_COLORS - 1) * SWATCH_GAP)
#define SWATCH_X ((CANVAS_W - SWATCH_STRIPE_W) / 2)
#define SWATCH_Y 8

#define BG_R 10
#define BG_G 10
#define BG_B 12

typedef struct {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    const char *name;
} AcetateColor;

static const AcetateColor PALETTE[NUM_COLORS] = {
    {255, 255, 255, "Transparent (Clear)"},
    {245, 55, 65,   "Red"},
    {255, 140, 30,  "Orange"},
    {255, 225, 45,  "Yellow"},
    {60, 215, 85,   "Green"},
    {40, 215, 245,  "Cyan"},
    {55, 125, 255,  "Blue"},
    {235, 70, 205,  "Magenta"}
};

/* 4x4 font glyph lookup: bit 0 is left pixel.
 * Matches tools/fw/text.c and img/4x4_font.png. */
static const Uint8 *glyph_4x4(char c)
{
    static const Uint8 digits[10][4] = {
        {0x07, 0x05, 0x07, 0x00}, /* 0 */
        {0x03, 0x02, 0x02, 0x00}, /* 1 */
        {0x03, 0x02, 0x06, 0x00}, /* 2 */
        {0x07, 0x06, 0x07, 0x00}, /* 3 */
        {0x05, 0x07, 0x04, 0x00}, /* 4 */
        {0x06, 0x02, 0x03, 0x00}, /* 5 */
        {0x01, 0x07, 0x07, 0x00}, /* 6 */
        {0x07, 0x04, 0x04, 0x00}, /* 7 */
        {0x02, 0x07, 0x07, 0x00}, /* 8 */
        {0x07, 0x07, 0x04, 0x00}, /* 9 */
    };
    static const Uint8 letters[26][4] = {
        {0x06, 0x05, 0x07, 0x00}, /* A */
        {0x01, 0x07, 0x07, 0x00}, /* B */
        {0x07, 0x01, 0x07, 0x00}, /* C */
        {0x04, 0x07, 0x07, 0x00}, /* D */
        {0x07, 0x03, 0x07, 0x00}, /* E */
        {0x07, 0x03, 0x01, 0x00}, /* F */
        {0x03, 0x05, 0x07, 0x00}, /* G */
        {0x05, 0x07, 0x05, 0x00}, /* H */
        {0x07, 0x02, 0x07, 0x00}, /* I */
        {0x07, 0x02, 0x03, 0x00}, /* J */
        {0x05, 0x03, 0x05, 0x00}, /* K */
        {0x01, 0x01, 0x07, 0x00}, /* L */
        {0x03, 0x06, 0x04, 0x00}, /* M */
        {0x07, 0x05, 0x05, 0x00}, /* N */
        {0x07, 0x05, 0x07, 0x00}, /* O */
        {0x07, 0x07, 0x01, 0x00}, /* P */
        {0x07, 0x07, 0x04, 0x00}, /* Q */
        {0x07, 0x03, 0x05, 0x00}, /* R */
        {0x06, 0x02, 0x03, 0x00}, /* S */
        {0x07, 0x02, 0x02, 0x00}, /* T */
        {0x05, 0x05, 0x07, 0x00}, /* U */
        {0x05, 0x05, 0x02, 0x00}, /* V */
        {0x01, 0x03, 0x06, 0x00}, /* W */
        {0x05, 0x02, 0x05, 0x00}, /* X */
        {0x05, 0x02, 0x02, 0x00}, /* Y */
        {0x03, 0x02, 0x06, 0x00}, /* Z */
    };
    static const Uint8 dot[4] = {0x00, 0x00, 0x01, 0x00};
    static const Uint8 comma[4] = {0x00, 0x00, 0x02, 0x01};
    static const Uint8 colon[4] = {0x02, 0x00, 0x02, 0x00};
    static const Uint8 semicolon[4] = {0x02, 0x00, 0x02, 0x01};
    static const Uint8 dash[4] = {0x00, 0x07, 0x00, 0x00};
    static const Uint8 plus[4] = {0x02, 0x07, 0x02, 0x00};
    static const Uint8 equals[4] = {0x07, 0x00, 0x07, 0x00};
    static const Uint8 slash[4] = {0x04, 0x02, 0x01, 0x00};
    static const Uint8 blank[4] = {0x00, 0x00, 0x00, 0x00};

    if (c >= '0' && c <= '9') return digits[c - '0'];
    if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
    if (c >= 'A' && c <= 'Z') return letters[c - 'A'];
    if (c == '.') return dot;
    if (c == ',') return comma;
    if (c == ':') return colon;
    if (c == ';') return semicolon;
    if (c == '-') return dash;
    if (c == '+') return plus;
    if (c == '=') return equals;
    if (c == '/') return slash;
    if (c == ' ') return blank;
    return NULL;
}

static void draw_text_4x4(SDL_Renderer *renderer, int x, int y, const char *s,
                          Uint8 r, Uint8 g, Uint8 b)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    for (; *s != '\0'; s++) {
        const Uint8 *rows = glyph_4x4(*s);
        if (rows) {
            for (int row = 0; row < 4; row++) {
                for (int col = 0; col < 4; col++) {
                    if ((rows[row] >> col) & 1)
                        SDL_RenderDrawPoint(renderer, x + col, y + row);
                }
            }
        }
        x += 4;
    }
}

typedef struct {
    int x;
    int y;
} Point;

static Uint8 leds[GRID][GRID];
static Uint8 acetate[GRID][GRID];
static int active_color = 1; // default to Red
static int copied_timer = 0; // frames to display "COPIED" notification

static int get_line_points(int x0, int y0, int x1, int y1, Point *pts, int max_pts)
{
    int count = 0;
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (1) {
        if (x0 >= 0 && x0 < GRID && y0 >= 0 && y0 < GRID) {
            if (count < max_pts) {
                pts[count].x = x0;
                pts[count].y = y0;
                count++;
            }
        }
        if (x0 == x1 && y0 == y1)
            break;
        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
    return count;
}

static void flood_fill_leds(int start_x, int start_y)
{
    Uint8 target = leds[start_y][start_x];
    Uint8 replacement = !target;

    int qx[GRID * GRID];
    int qy[GRID * GRID];
    int head = 0;
    int tail = 0;
    bool visited[GRID][GRID] = {{false}};

    qx[tail] = start_x;
    qy[tail] = start_y;
    tail++;
    visited[start_y][start_x] = true;

    while (head < tail) {
        int cx = qx[head];
        int cy = qy[head];
        head++;

        leds[cy][cx] = replacement;

        const int dx[] = {1, -1, 0, 0};
        const int dy[] = {0, 0, 1, -1};
        for (int i = 0; i < 4; i++) {
            int nx = cx + dx[i];
            int ny = cy + dy[i];
            if (nx >= 0 && nx < GRID && ny >= 0 && ny < GRID) {
                if (!visited[ny][nx] && leds[ny][nx] == target) {
                    visited[ny][nx] = true;
                    qx[tail] = nx;
                    qy[tail] = ny;
                    tail++;
                }
            }
        }
    }
}

static void flood_fill_acetate(int start_x, int start_y, Uint8 new_color)
{
    Uint8 target = acetate[start_y][start_x];
    if (target == new_color)
        return;

    int qx[GRID * GRID];
    int qy[GRID * GRID];
    int head = 0;
    int tail = 0;
    bool visited[GRID][GRID] = {{false}};

    qx[tail] = start_x;
    qy[tail] = start_y;
    tail++;
    visited[start_y][start_x] = true;

    while (head < tail) {
        int cx = qx[head];
        int cy = qy[head];
        head++;

        acetate[cy][cx] = new_color;

        const int dx[] = {1, -1, 0, 0};
        const int dy[] = {0, 0, 1, -1};
        for (int i = 0; i < 4; i++) {
            int nx = cx + dx[i];
            int ny = cy + dy[i];
            if (nx >= 0 && nx < GRID && ny >= 0 && ny < GRID) {
                if (!visited[ny][nx] && acetate[ny][nx] == target) {
                    visited[ny][nx] = true;
                    qx[tail] = nx;
                    qy[tail] = ny;
                    tail++;
                }
            }
        }
    }
}

static SDL_Texture *create_fallback_led(SDL_Renderer *renderer, bool is_on)
{
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, 4, 4, 32, SDL_PIXELFORMAT_RGBA32);
    if (!surface)
        return NULL;

    Uint32 *pixels = (Uint32 *)surface->pixels;
    Uint32 trans = SDL_MapRGBA(surface->format, 0, 0, 0, 0);
    Uint32 color = is_on ? SDL_MapRGBA(surface->format, 255, 255, 255, 255)
                         : SDL_MapRGBA(surface->format, 51, 51, 51, 255);

    for (int i = 0; i < 16; i++)
        pixels[i] = trans;

    // 2x2 centered dot
    pixels[1 * 4 + 1] = color;
    pixels[1 * 4 + 2] = color;
    pixels[2 * 4 + 1] = color;
    pixels[2 * 4 + 2] = color;

    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return tex;
}

static SDL_Texture *load_led_tex(SDL_Renderer *renderer, const char *file, bool is_on)
{
    const char *paths[] = {
        "../sim/img/",
        "tools/sim/img/",
        "sim/img/",
        "img/",
        NULL
    };
    char full[512];
    SDL_Surface *surface = NULL;

    for (int i = 0; paths[i] != NULL; i++) {
        snprintf(full, sizeof(full), "%s%s", paths[i], file);
        surface = IMG_Load(full);
        if (surface)
            break;
    }

    if (!surface)
        return create_fallback_led(renderer, is_on);

    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return tex ? tex : create_fallback_led(renderer, is_on);
}

static void draw_checkerboard(SDL_Renderer *renderer, int x, int y, int w, int h)
{
    for (int py = 0; py < h; py++) {
        for (int px = 0; px < w; px++) {
            if (((px / 2) + (py / 2)) % 2 == 0)
                SDL_SetRenderDrawColor(renderer, 205, 205, 210, 255);
            else
                SDL_SetRenderDrawColor(renderer, 95, 95, 105, 255);
            SDL_RenderDrawPoint(renderer, x + px, y + py);
        }
    }
}

static void draw_copied_badge(SDL_Renderer *renderer)
{
    // Draw 4x4 pixel font "COPIED" (24 pixels wide, 4 pixels tall)
    int start_x = (CANVAS_W - 24) / 2;
    int start_y = 94;
    draw_text_4x4(renderer, start_x, start_y, "COPIED", 120, 240, 150);
}

static void render_scene(SDL_Renderer *renderer, SDL_Texture *led_on, SDL_Texture *led_off,
                         bool line_active, int lx0, int ly0, int lx1, int ly1,
                         Uint8 l_button, Uint8 l_led_state)
{
    // Background
    SDL_SetRenderDrawColor(renderer, BG_R, BG_G, BG_B, 255);
    SDL_RenderClear(renderer);

    // Grid bezel / border
    SDL_Rect bezel = {FIELD_X - 1, FIELD_Y - 1, FIELD_SIZE + 2, FIELD_SIZE + 2};
    SDL_SetRenderDrawColor(renderer, 24, 24, 28, 255);
    SDL_RenderDrawRect(renderer, &bezel);

    // Build visible state (including live line preview if active)
    Uint8 cur_leds[GRID][GRID];
    Uint8 cur_acetate[GRID][GRID];
    memcpy(cur_leds, leds, sizeof(leds));
    memcpy(cur_acetate, acetate, sizeof(acetate));

    if (line_active) {
        Point pts[64];
        int count = get_line_points(lx0, ly0, lx1, ly1, pts, 64);
        for (int i = 0; i < count; i++) {
            int px = pts[i].x;
            int py = pts[i].y;
            if (l_button == SDL_BUTTON_LEFT)
                cur_leds[py][px] = l_led_state;
            else if (l_button == SDL_BUTTON_RIGHT)
                cur_acetate[py][px] = (Uint8)active_color;
        }
    }

    // LEDs and acetate layers
    for (int y = 0; y < GRID; y++) {
        for (int x = 0; x < GRID; x++) {
            SDL_Rect cell = {FIELD_X + x * LED, FIELD_Y + y * LED, LED, LED};
            Uint8 c_idx = cur_acetate[y][x];
            bool on = cur_leds[y][x] != 0;

            if (on) {
                if (c_idx == 0) {
                    // Transparent acetate: pure white LED
                    SDL_SetTextureColorMod(led_on, 255, 255, 255);
                    SDL_RenderCopy(renderer, led_on, NULL, &cell);
                } else {
                    // Filtered LED: color modulated by acetate
                    const AcetateColor *ac = &PALETTE[c_idx];
                    SDL_SetTextureColorMod(led_on, ac->r, ac->g, ac->b);
                    SDL_RenderCopy(renderer, led_on, NULL, &cell);

                    // Cell glow wash when lit
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(renderer, ac->r, ac->g, ac->b, 50);
                    SDL_RenderFillRect(renderer, &cell);
                }
            } else {
                if (c_idx > 0) {
                    // Show unlit LED dot through the colored acetate with subtle tint
                    const AcetateColor *ac = &PALETTE[c_idx];
                    SDL_SetTextureColorMod(
                        led_off,
                        (Uint8)(160 + ac->r / 3),
                        (Uint8)(160 + ac->g / 3),
                        (Uint8)(160 + ac->b / 3)
                    );
                    SDL_RenderCopy(renderer, led_off, NULL, &cell);

                    // Clean, transparent acetate film wash
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(renderer, ac->r, ac->g, ac->b, 28);
                    SDL_RenderFillRect(renderer, &cell);
                } else {
                    SDL_SetTextureColorMod(led_off, 255, 255, 255);
                    SDL_RenderCopy(renderer, led_off, NULL, &cell);
                }
            }
        }
    }

    // Top acetate colors stripe
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    for (int i = 0; i < NUM_COLORS; i++) {
        int sx = SWATCH_X + i * (SWATCH_W + SWATCH_GAP);
        int sy = SWATCH_Y;
        SDL_Rect srect = {sx, sy, SWATCH_W, SWATCH_H};

        if (i == 0) {
            // Symmetrical 4x4 checkerboard (8x8 swatch)
            draw_checkerboard(renderer, sx, sy, SWATCH_W, SWATCH_H);
        } else {
            SDL_SetRenderDrawColor(renderer, PALETTE[i].r, PALETTE[i].g, PALETTE[i].b, 255);
            SDL_RenderFillRect(renderer, &srect);
        }

        // Swatch frame
        SDL_SetRenderDrawColor(renderer, 45, 45, 55, 255);
        SDL_RenderDrawRect(renderer, &srect);

        // Highlight active color
        if (i == active_color) {
            SDL_Rect select_rect = {sx - 1, sy - 1, SWATCH_W + 2, SWATCH_H + 2};
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &select_rect);
        }
    }

    // Display "COPIED" notification if recently screenshotted
    if (copied_timer > 0) {
        copied_timer--;
        draw_copied_badge(renderer);
    }
}

static void save_and_copy_screenshot(SDL_Renderer *renderer, int win_w, int win_h)
{
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, win_w, win_h, 32, SDL_PIXELFORMAT_RGBA32);
    if (!surface)
        return;

    if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA32, surface->pixels, surface->pitch) == 0) {
        const char *tmp_file = "/tmp/acetate_sim_screenshot.png";
        IMG_SavePNG(surface, tmp_file);
        IMG_SavePNG(surface, "acetate_screenshot.png");

        char cmd[512];
        snprintf(cmd, sizeof(cmd), "python3 tools/acetate_sim/copy_clip.py \"%s\" >/dev/null 2>&1 &", tmp_file);
        int rc = system(cmd);
        (void)rc;

        copied_timer = 90; // Show "COPIED" for ~1.5s at 60Hz
    }
    SDL_FreeSurface(surface);
}

static void present_canvas(SDL_Renderer *renderer, SDL_Texture *canvas, int *out_scale, SDL_Rect *out_dst, int *out_w, int *out_h)
{
    int win_w;
    int win_h;
    SDL_SetRenderTarget(renderer, NULL);
    SDL_GetRendererOutputSize(renderer, &win_w, &win_h);

    int scale_x = win_w / CANVAS_W;
    int scale_y = win_h / CANVAS_H;
    int scale = (scale_x < scale_y) ? scale_x : scale_y;
    if (scale < 1)
        scale = 1;

    SDL_Rect dst;
    dst.w = CANVAS_W * scale;
    dst.h = CANVAS_H * scale;
    dst.x = (win_w - dst.w) / 2;
    dst.y = (win_h - dst.h) / 2;

    *out_scale = scale;
    *out_dst = dst;
    *out_w = win_w;
    *out_h = win_h;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, canvas, NULL, &dst);
}

int main(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    IMG_Init(IMG_INIT_PNG);

    int initial_scale = 6;
    SDL_Window *window = SDL_CreateWindow(
        "Maki-Nita Acetate Simulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        CANVAS_W * initial_scale,
        CANVAS_H * initial_scale,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN
    );
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"); // Crisp pixel art

    SDL_Texture *canvas = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        CANVAS_W,
        CANVAS_H
    );
    if (!canvas) {
        fprintf(stderr, "Canvas creation failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Texture *led_on = load_led_tex(renderer, "led_on.png", true);
    SDL_Texture *led_off = load_led_tex(renderer, "led_off.png", false);

    bool running = true;
    bool left_down = false;
    bool right_down = false;
    bool holding_f = false;
    bool holding_l = false;

    bool line_active = false;
    int line_start_x = 0;
    int line_start_y = 0;
    int line_end_x = 0;
    int line_end_y = 0;
    Uint8 line_button = 0;
    Uint8 line_led_state = 1;

    bool trigger_screenshot = false;
    Uint8 paint_led_state = 1;
    int scale = initial_scale;
    int win_w = CANVAS_W * scale;
    int win_h = CANVAS_H * scale;
    SDL_Rect dst_rect = {0, 0, CANVAS_W * scale, CANVAS_H * scale};

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                case SDLK_q:
                    running = false;
                    break;
                case SDLK_f:
                    holding_f = true;
                    break;
                case SDLK_l:
                    holding_l = true;
                    break;
                case SDLK_s:
                    // Screenshot & clipboard on 's' or 'Ctrl + s'
                    trigger_screenshot = true;
                    break;
                case SDLK_1:
                case SDLK_2:
                case SDLK_3:
                case SDLK_4:
                case SDLK_5:
                case SDLK_6:
                case SDLK_7:
                case SDLK_8:
                    active_color = event.key.keysym.sym - SDLK_1;
                    break;
                case SDLK_c:
                    // Clear all LEDs
                    for (int y = 0; y < GRID; y++)
                        for (int x = 0; x < GRID; x++)
                            leds[y][x] = 0;
                    break;
                case SDLK_a:
                    // Clear all acetate (set to transparent)
                    for (int y = 0; y < GRID; y++)
                        for (int x = 0; x < GRID; x++)
                            acetate[y][x] = 0;
                    break;
                default:
                    break;
                }
                break;

            case SDL_KEYUP:
                if (event.key.keysym.sym == SDLK_f)
                    holding_f = false;
                if (event.key.keysym.sym == SDLK_l) {
                    holding_l = false;
                    if (line_active) {
                        Point pts[64];
                        int count = get_line_points(line_start_x, line_start_y, line_end_x, line_end_y, pts, 64);
                        for (int i = 0; i < count; i++) {
                            if (line_button == SDL_BUTTON_LEFT)
                                leds[pts[i].y][pts[i].x] = line_led_state;
                            else if (line_button == SDL_BUTTON_RIGHT)
                                acetate[pts[i].y][pts[i].x] = (Uint8)active_color;
                        }
                        line_active = false;
                    }
                }
                break;

            case SDL_MOUSEBUTTONDOWN: {
                int mx = event.button.x;
                int my = event.button.y;
                int cx = (mx - dst_rect.x) / scale;
                int cy = (my - dst_rect.y) / scale;

                // Check swatch strip
                if (cy >= SWATCH_Y && cy < SWATCH_Y + SWATCH_H) {
                    for (int i = 0; i < NUM_COLORS; i++) {
                        int sx = SWATCH_X + i * (SWATCH_W + SWATCH_GAP);
                        if (cx >= sx && cx < sx + SWATCH_W) {
                            active_color = i;
                            break;
                        }
                    }
                }

                // Check LED grid
                if (cx >= FIELD_X && cx < FIELD_X + FIELD_SIZE &&
                    cy >= FIELD_Y && cy < FIELD_Y + FIELD_SIZE) {
                    int gx = (cx - FIELD_X) / LED;
                    int gy = (cy - FIELD_Y) / LED;

                    if (holding_f) {
                        // Flood fill
                        if (event.button.button == SDL_BUTTON_LEFT)
                            flood_fill_leds(gx, gy);
                        else if (event.button.button == SDL_BUTTON_RIGHT)
                            flood_fill_acetate(gx, gy, (Uint8)active_color);
                    } else if (holding_l) {
                        // Start straight line
                        line_active = true;
                        line_start_x = gx;
                        line_start_y = gy;
                        line_end_x = gx;
                        line_end_y = gy;
                        line_button = event.button.button;
                        line_led_state = !leds[gy][gx];
                    } else {
                        // Regular click & drag paint
                        if (event.button.button == SDL_BUTTON_LEFT) {
                            left_down = true;
                            leds[gy][gx] = !leds[gy][gx];
                            paint_led_state = leds[gy][gx];
                        } else if (event.button.button == SDL_BUTTON_RIGHT) {
                            right_down = true;
                            acetate[gy][gx] = (Uint8)active_color;
                        }
                    }
                }
                break;
            }

            case SDL_MOUSEBUTTONUP:
                if (line_active && event.button.button == line_button) {
                    Point pts[64];
                    int count = get_line_points(line_start_x, line_start_y, line_end_x, line_end_y, pts, 64);
                    for (int i = 0; i < count; i++) {
                        if (line_button == SDL_BUTTON_LEFT)
                            leds[pts[i].y][pts[i].x] = line_led_state;
                        else if (line_button == SDL_BUTTON_RIGHT)
                            acetate[pts[i].y][pts[i].x] = (Uint8)active_color;
                    }
                    line_active = false;
                }
                if (event.button.button == SDL_BUTTON_LEFT)
                    left_down = false;
                else if (event.button.button == SDL_BUTTON_RIGHT)
                    right_down = false;
                break;

            case SDL_MOUSEMOTION: {
                int mx = event.motion.x;
                int my = event.motion.y;
                int cx = (mx - dst_rect.x) / scale;
                int cy = (my - dst_rect.y) / scale;

                int gx = (cx - FIELD_X) / LED;
                int gy = (cy - FIELD_Y) / LED;
                if (gx < 0) gx = 0;
                if (gx >= GRID) gx = GRID - 1;
                if (gy < 0) gy = 0;
                if (gy >= GRID) gy = GRID - 1;

                if (line_active) {
                    line_end_x = gx;
                    line_end_y = gy;
                } else if (cx >= FIELD_X && cx < FIELD_X + FIELD_SIZE &&
                           cy >= FIELD_Y && cy < FIELD_Y + FIELD_SIZE) {
                    if (left_down)
                        leds[gy][gx] = paint_led_state;
                    if (right_down)
                        acetate[gy][gx] = (Uint8)active_color;
                }
                break;
            }
            }
        }

        // Render scene to canvas texture
        SDL_SetRenderTarget(renderer, canvas);
        render_scene(renderer, led_on, led_off,
                     line_active, line_start_x, line_start_y, line_end_x, line_end_y,
                     line_button, line_led_state);

        // Present canvas with integer pixel scale to window
        present_canvas(renderer, canvas, &scale, &dst_rect, &win_w, &win_h);

        // If screenshot was requested, capture right after rendering the full app
        if (trigger_screenshot) {
            save_and_copy_screenshot(renderer, win_w, win_h);
            trigger_screenshot = false;
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(led_on);
    SDL_DestroyTexture(led_off);
    SDL_DestroyTexture(canvas);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
