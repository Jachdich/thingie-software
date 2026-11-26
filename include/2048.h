#include "drawing.h"

#ifndef __2048_H
#define __2048_H

struct GameState;

void two048_init(struct GameState *state);
bool two048_step(struct GameState *state, Screen s);

#endif
