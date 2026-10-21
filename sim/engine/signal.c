#include "engine/signal.h"

#include <stdio.h>

void net_init(Net *net, const char *name, Sig pull)
{
    int i;

    net->name = name;
    net->npin = 0;
    net->pull = pull;
    net->level = pull;
    for (i = 0; i < NET_MAX_PINS; i++)
        net->pins[i] = 0;
}

void net_tie(Net *net, Pin *pin)
{
    if (net->npin >= NET_MAX_PINS) {
        fprintf(stderr, "net %s is full\n", net->name);
        return;
    }
    net->pins[net->npin++] = pin;
    pin->net = net;
}

void net_settle(Net *net)
{
    Sig acc = SIG_Z;
    int i;

    for (i = 0; i < net->npin; i++) {
        Sig drive = net->pins[i]->drive;

        if (drive == SIG_Z)
            continue;
        if (acc == SIG_Z || acc == drive)
            acc = drive;
        else
            acc = SIG_X;
    }
    if (acc == SIG_Z)
        acc = net->pull;
    net->level = acc;
    for (i = 0; i < net->npin; i++)
        net->pins[i]->level = acc;
}
