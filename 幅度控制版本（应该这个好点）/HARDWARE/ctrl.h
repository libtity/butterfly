#ifndef C8T6_CTRL_H
#define C8T6_CTRL_H

#include "bsp.h"

#define WING_SHUT_L 50
#define WING_SHUT_R 250

#define WING_UP_L 90
#define WING_DOWN_L 170
#define WING_UP_R 210
#define WING_DOWN_R 130

void set_left(unsigned int x);

void set_right(unsigned int x);

void count(void);

void keep(void);

#endif //C8T6_CTRL_H
