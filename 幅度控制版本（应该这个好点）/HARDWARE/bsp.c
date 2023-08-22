#include "bsp.h"

void init(void) {
    delay_init();
    PWM_Init();
    SBUS_USART1_Config(100000);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}

void loop(void) {
    // ÎåÍ¨µÀÏòÉÏ£¬´¦ÓÚÒ£¿ØÐÞ¸Ä×´Ì¬
    if (SBUS_Ch[5] == 200) {
        count();
    }
        // ÎåÍ¨µÀÏòÏÂ£¬´¦ÓÚ¾²Ö¹×´Ì¬
    else if (SBUS_Ch[5] == 1800) {
        set_left(WING_SHUT_L);
        set_right(WING_SHUT_R);
        delay_ms(100);
    }
        // ÎåÍ¨µÀÖÐ¼äÎ»ÖÃ£¬ÎÞ²Ù×÷
//    else {
//        keep();
//    }
}

