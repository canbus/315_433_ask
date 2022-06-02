
#include "ask_hal.h"
#include "main.h"

ask_t   ask315;

//##################################################################################
#define RX315_PIN           GPIO_PIN_7
#define RX315_GPIO          GPIOA
#define TX315_PIN           GPIO_PIN_6
#define TX315_GPIO          GPIOA
extern TIM_HandleTypeDef htim2;  // config in cubemx, 1 us tick, use 32 bit timer
//##################################################################################
inline void ask_write_pin_315(bool data)
{
    HAL_GPIO_WritePin(TX315_GPIO, TX315_PIN, (GPIO_PinState)data);
}
//##################################################################################
inline bool ask_read_pin_315(void)
{
    return HAL_GPIO_ReadPin(RX315_GPIO, RX315_PIN);
}
//##################################################################################
inline uint32_t ask_micros_315(void)
{
    return htim2.Instance->CNT ;
}
void ask_restart_timer(void)
{
    htim2.Instance->CNT = 0;
    HAL_TIM_Base_Start(&htim2);
}
//##################################################################################
void ask_init_rx315(void)
{
    //  config gpio and timer in cubemx        
    HAL_TIM_Base_Start(&htim2);
}
//##################################################################################
void ask_init_tx315(void)
{
    //  config in cubemx
}
//##################################################################################
inline void ask_delay_ms_315(uint32_t delay_ms)
{
    HAL_Delay(delay_ms);
}
//##################################################################################
inline void ask_delay_us_315(uint32_t delay_us)
{
    uint32_t time = htim2.Instance->CNT ;
    while (htim2.Instance->CNT - time < delay_us);
}
//##################################################################################
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == RX315_PIN)
  {
      ask_pinchange_callback(&ask315);
  }
}
