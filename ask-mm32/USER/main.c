
#define _MAIN_C_

#include "main.h"

#include "ask.h"
#include "ask_hal.h"
#include "tim2.h"
#include "uart.h"

#define REPEATE_SEND 1

extern u32 sysTick_ms;
extern u32 SystemCoreClock;

typedef struct {
    uint8_t data[_ASK_MAX_BYTE_LEN_ + 1];
    uint8_t confirm;
    uint8_t available;
    uint8_t newData;
    uint32_t tick;
} ask_data_t;

uint8_t sn = 0;

void ask_main(void *argument)
{
    ask_data_t ask_data = {0};
    ask315.fn_init_rx = ask_init_rx315;
    ask315.fn_init_tx = ask_init_tx315;
    ask315.fn_micros = ask_micros_315;
    ask315.fn_read_pin = ask_read_pin_315;
    ask315.fn_write_pin = ask_write_pin_315;
    ask315.fn_delay_ms = ask_delay_ms_315;
    ask315.fn_delay_us = ask_delay_us_315;
    ask315.fn_restat_tm = ask_restart_timer;
    ask_init(&ask315);
    ask315.enable_rx = true;
    for (;;) {
        HAL_Delay(1);
        if (ask_available(&ask315)) {
            uint8_t data[_ASK_MAX_BYTE_LEN_ + 1]={0};

#if REPEATE_SEND
            ask_read_bytes(&ask315, data);
            uint8_t sum = 0xAA + sn + data[0] + data[1] + data[2];
            printf("%c%c%s%c", '\xAA', sn++, data, sum);
#else
            if (sysTick_ms - ask_data.tick > 200)
                ask_data.newData = 1;

            ask_read_bytes(&ask315, data);
            ask_data.tick = sysTick_ms;

            if (ask_data.confirm == 0) {
                memcpy(&ask_data.data, data, _ASK_MAX_BYTE_LEN_);
                ask_data.confirm = 1;
            } else {
                if (strncmp(&ask_data.data, data, _ASK_MAX_BYTE_LEN_) == 0) {
                    ask_data.available = 1;
                } else {
                    memcpy(&ask_data.data, data, _ASK_MAX_BYTE_LEN_);
                }
            }
            if (ask_data.available && ask_data.newData) {
                ask_data.newData = 0;
                uint8_t sum = 0xAA + sn + ask_data.data[0] + ask_data.data[1] + ask_data.data[2];
                printf("%c%c%s%c", '\xAA', sn++, data, sum);
            }
#endif
            // ask_wait(&ask315);
            // printf("%s", data);
            // ask_send_bytes(&ask315,"123",3,100,3);
        }
    }
}

s32 main(void)
{
    u32 uiCnt = 0;
    CONSOLE_Init(115200);
    GPIOInit();
    // TIM2_UPCount_Init(100 - 1, SystemCoreClock / 100000 - 1);
    TIM2_UPCount_Init(72 - 1, 0xFFFFFFFF - 1);  // 10us
    TIM3_UPCount_Init(720 - 1, 100 - 1);        // 1ms
    // printf("SystemCoreClock:%d\n", SystemCoreClock);
    ask_main(NULL);
    while (1) {
        if (sysTick_ms % 10 == 0) {
            // ucTim2Flag = 0;
            printf("us:%d,ms:%d\r\n", ucTim2Flag, sysTick_ms);
            // ask_restart_timer();
            // uiCnt++;
        }
    }
}
