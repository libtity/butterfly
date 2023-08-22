#include "ctrl.h"

void set_left(unsigned int x) {
    TIM1->CCR1 = x;
}

void set_right(unsigned int x) {
    TIM1->CCR4 = x;
}

int speed = 500;
int turn = 0;

void count(void) {
    // Ò£¿ØÐÅºÅ´«Êä
    /**
     * speedÎª×óÊú£¬1800-1000-200£¬¿ØÖÆÆµÂÊ£¬ÓÃÓÚÑÓ³Ù£¬µ¥Î»ºÁÃë
     * turnÎªÓÒºá£¬1400-1000-600£¬³ýÒÔ20Ö®ºó£¬ÄÜ¿ØÖÆ20µÄµ¥Î»
     * left 250-190-90-50
     * right 50-90-190-250
     */

	speed = 120 - (SBUS_Ch[3] - 200) / 16;
    turn = (SBUS_Ch[1] - 1000) / 20;

    set_left(WING_UP_L + turn);
    set_right(WING_UP_R + turn);

    delay_ms(speed);

    set_left(WING_DOWN_L - turn);
    set_right(WING_DOWN_R - turn);

    delay_ms(speed);
}

void keep(void){
    set_left(WING_UP_L + turn);
    set_right(WING_UP_R + turn);

    delay_ms(speed);

    set_left(WING_DOWN_L - turn);
    set_right(WING_DOWN_R - turn);

    delay_ms(speed);
}
