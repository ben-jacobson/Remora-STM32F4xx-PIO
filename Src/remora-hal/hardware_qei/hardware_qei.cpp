#include "hardware_qei.h"

hardware_qei::hardware_qei() :
    qeiIndex(NC)
{
    this->hasIndex = false;
    this->init();
}


hardware_qei::hardware_qei(bool hasIndex) :
    hasIndex(hasIndex),
    qeiIndex(PE_13)
{
    this->hasIndex = true;
    this->irq = EXTI15_10_IRQn;

    this->init();

    qeiIndex.rise(callback(this, &hardware_qei::interruptHandler));
    //NVIC_EnableIRQ(this->irq);
    HAL_NVIC_SetPriority(this->irq, 0, 0);
}


void hardware_qei::interruptHandler()
{
    this->indexDetected = true;
    this->indexCount = this->get();
}


uint32_t hardware_qei::get()
{
    return __HAL_TIM_GET_COUNTER(ptr_tim_handler);
}

void hardware_qei::init()
{
    printf("  Initialising hardware QEI module\n");
    // TODO - replace with Remora GPIO initilasation. 
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_TIM1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /**TIM1 GPIO Configuration
    PA8     ------> TIM1_CH1
    PA9     ------> TIM1_CH2
    */

    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Set up the timers
    ptr_tim_handler->Instance = TIM1;

    ptr_tim_handler->Init.Prescaler = 0;
    ptr_tim_handler->Init.CounterMode = TIM_COUNTERMODE_UP;
    ptr_tim_handler->Init.Period = 65535;
    ptr_tim_handler->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    ptr_tim_handler->Init.RepetitionCounter = 0;
    ptr_tim_handler->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
    sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
    sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
    sConfig.IC1Filter = 10;

    sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
    sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
    sConfig.IC2Filter = 10;

    if (HAL_TIM_Encoder_Init(ptr_tim_handler, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

    if (HAL_TIMEx_MasterConfigSynchronization(ptr_tim_handler, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }


    if (HAL_TIM_Encoder_Start(ptr_tim_handler, TIM_CHANNEL_2)!=HAL_OK)
    {
        printf("Couldn't Start Encoder\r\n");
    }
}