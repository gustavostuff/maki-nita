#ifndef RD_SIGNAL_H
#define RD_SIGNAL_H

/*
 * Pin levels for the board netlist.
 * A driver forces L or H, or releases the net with Z.
 * X is a fight between two drivers. A pull sets the level when every driver is Z.
 */

typedef enum Sig {
    SIG_Z = 0,
    SIG_L = 1,
    SIG_H = 2,
    SIG_X = 3
} Sig;

typedef enum PinDir {
    PIN_IN = 0,
    PIN_OUT = 1,
    PIN_PWR = 2,
    PIN_NC = 3
} PinDir;

#define NET_MAX_PINS 24

typedef struct Net Net;

typedef struct Pin {
    const char *name;
    int number; /* package pin, starting at 1 */
    PinDir dir;
    Sig drive;
    Sig level;
    Net *net;
} Pin;

struct Net {
    const char *name;
    Pin *pins[NET_MAX_PINS];
    int npin;
    Sig pull;
    Sig level;
};

void net_init(Net *net, const char *name, Sig pull);
void net_tie(Net *net, Pin *pin);
void net_settle(Net *net);

#endif
