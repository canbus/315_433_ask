
// Files includes
#include "exti.h"
#include "delay.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

typedef struct 
{
    int RCC_AHBPeriph_GPIOx;
    GPIO_TypeDef *GPIOx;
    int GPIO_PINS[16];//最后一个置成-1
    int GPIO_Mode;
}GPIO_CFG_T;


GPIO_CFG_T gpio_cfg_input={   .RCC_AHBPeriph_GPIOx = RCC_AHBPeriph_GPIOA,
							.GPIOx = GPIOA,
						    .GPIO_PINS = {GPIO_Pin_7,-1},
						    .GPIO_Mode = GPIO_Mode_IPU };
GPIO_CFG_T gpio_cfg_output={  .RCC_AHBPeriph_GPIOx = RCC_AHBPeriph_GPIOA,
							.GPIOx = GPIOA,
						    .GPIO_PINS = {GPIO_Pin_6,-1},
						    .GPIO_Mode = GPIO_Mode_Out_PP };

static void _gpioInit(GPIO_CFG_T *cfg)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHBPeriphClockCmd(cfg->RCC_AHBPeriph_GPIOx, ENABLE);

    GPIO_InitStructure.GPIO_Pin = 0;
    for(int i=0; cfg->GPIO_PINS[i] > 0; i++){
        GPIO_InitStructure.GPIO_Pin  |= cfg->GPIO_PINS[i]; 
    }
               
    GPIO_InitStructure.GPIO_Mode = cfg->GPIO_Mode;                               //set as pull down input
    GPIO_Init(cfg->GPIOx, &GPIO_InitStructure);
}


static void EXTI_NVIC_Init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_15_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 0x02;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // NVIC_InitStructure.NVIC_IRQChannel = EXTI0_1_IRQn;
    // NVIC_InitStructure.NVIC_IRQChannelPriority = 0x01;
    // NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    // NVIC_Init(&NVIC_InitStructure);

    // NVIC_InitStructure.NVIC_IRQChannel = EXTI2_3_IRQn;
    // NVIC_InitStructure.NVIC_IRQChannelPriority = 0x03;
    // NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    // NVIC_Init(&NVIC_InitStructure);

}

static void EXTI_NVIC_Config(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource7);
    EXTI_InitStructure.EXTI_Line = EXTI_Line7;
    EXTI_Init(&EXTI_InitStructure);

    //PB4
    // SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource4);
    // EXTI_InitStructure.EXTI_Line = EXTI_Line4;
    // EXTI_Init(&EXTI_InitStructure);
}

void EXTI_Config(void)
{
    EXTI_NVIC_Config();
    EXTI_NVIC_Init();
}
#if 0
void EXTI0_1_IRQHandler(void)
{
    if(SET == EXTI_GetFlagStatus(EXTI_Line0)){
        EXTI_ClearFlag(EXTI_Line0);
        irqHandel(BTN_KEY1);
    }
    if(SET == EXTI_GetFlagStatus(EXTI_Line1)){
        EXTI_ClearFlag(EXTI_Line1);
        irqHandel(ENC_A);
    }
}

void EXTI2_3_IRQHandler(void)
{
    if(SET == EXTI_GetFlagStatus(EXTI_Line2)){
        EXTI_ClearFlag(EXTI_Line2);
        irqHandel(BTN_KEY2);
    }
    if(SET == EXTI_GetFlagStatus(EXTI_Line3)){
        EXTI_ClearFlag(EXTI_Line3);
        irqHandel(ENC_B);
    }
}
#endif
void EXTI4_15_IRQHandler(void)
{
    if(SET == EXTI_GetFlagStatus(EXTI_Line7)){
        HAL_GPIO_EXTI_Callback(EXTI_Line7);
        EXTI_ClearFlag(EXTI_Line7);
        // if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_7)){
        //     GPIO_SetBits(GPIOA,GPIO_Pin_6);
        // }else{
        //     GPIO_ResetBits(GPIOA,GPIO_Pin_6);
        // }
    }
}

void GPIOInit(void)
{
    // _gpioInit(&gpio_cfg_input);
    // _gpioInit(&gpio_cfg_output);
    
    GPIO_InitTypeDef gpio_InitDef;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

    gpio_InitDef.GPIO_Pin = GPIO_Pin_6;
    gpio_InitDef.GPIO_Mode = GPIO_Mode_Out_PP; 
    gpio_InitDef.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio_InitDef);

    gpio_InitDef.GPIO_Pin = GPIO_Pin_7;
    gpio_InitDef.GPIO_Mode = GPIO_Mode_IPD; 
    GPIO_Init(GPIOA, &gpio_InitDef);
    EXTI_Config();
}