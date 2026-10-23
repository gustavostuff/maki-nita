#include "sound.h"

#include "synth.h"

#include <SDL.h>

#define RING 8192

static uint16_t ring[RING];
static volatile unsigned rd;
static volatile unsigned wr;
static SDL_AudioDeviceID dev;
static int dev_rate;
static int spread;
static int frame_err;

void sound(uint8_t index)
{
    synth_play(index);
}

static void push(uint16_t sample)
{
    unsigned w = wr;

    if (w - rd >= RING)
        return;
    ring[w & (RING - 1)] = sample;
    __sync_synchronize();
    wr = w + 1;
}

static void fill(void *userdata, Uint8 *stream, int len)
{
    Sint16 *out = (Sint16 *)stream;
    int n = len / (int)sizeof(Sint16);
    int i;

    (void)userdata;
    for (i = 0; i < n; i++) {
        unsigned r = rd;
        int sample = 512;

        if (r != wr) {
            __sync_synchronize();
            sample = ring[r & (RING - 1)];
            __sync_synchronize();
            rd = r + 1;
        }
        sample = (sample - 512) * 64;
        if (sample > 32767)
            sample = 32767;
        if (sample < -32768)
            sample = -32768;
        out[i] = (Sint16)sample;
    }
}

void sound_open(void)
{
    SDL_AudioSpec want;
    SDL_AudioSpec have;
    SDL_AudioDeviceID id;

    synth_reset();
    rd = 0;
    wr = 0;
    spread = 0;
    frame_err = 0;
    dev_rate = 0;
    SDL_zero(want);
    want.freq = SYNTH_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 512;
    want.callback = fill;
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
        return;
    id = SDL_OpenAudioDevice(NULL, 0, &want, &have,
                             SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
    if (id == 0 || have.freq <= 0)
        return;
    dev = id;
    dev_rate = have.freq;
    SDL_PauseAudioDevice(dev, 0);
}

void sound_render(void)
{
    int n = SYNTH_FRAME;

    if (dev == 0)
        return;
    frame_err += SYNTH_RATE % 60;
    if (frame_err >= 60) {
        n++;
        frame_err -= 60;
    }
    while (n > 0) {
        uint16_t sample = synth_sample();
        int copies = dev_rate / SYNTH_RATE;

        spread += dev_rate % SYNTH_RATE;
        if (spread >= SYNTH_RATE) {
            copies++;
            spread -= SYNTH_RATE;
        }
        while (copies > 0) {
            push(sample);
            copies--;
        }
        n--;
    }
}

void sound_close(void)
{
    if (dev == 0)
        return;
    SDL_CloseAudioDevice(dev);
    dev = 0;
}
