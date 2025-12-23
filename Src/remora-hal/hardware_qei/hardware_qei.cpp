#include "hardware_qei.h"

hardware_qei::hardware_qei(bool _hasIndex) :
    hasIndex(_hasIndex),
{
    this->init();

    if (hasIndex) 
    {
        indexPortAndPin = "PC_7";
        indexPin = new Pin(indexPortAndPin, GPIO_MODE_IT_RISING, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH, 0);

        irqIndex = EXTI9_5_IRQn;

        IndexInterrupt = new ModuleInterrupt<hardware_qei>      // How do we set this to trigger only on rise? 
        (
            irqIndex,
            this,
            &hardware_qei::handleIndexInterrupt
        );        

        // Need to configure interrupt in irqHandler.h. Assumed boiler plate in there for now.

        HAL_NVIC_SetPriority(this->irqIndex, 0, 0);
        HAL_NVIC_EnableIRQ(this->irqIndex);        
    }
}

void hardware_qei::handleIndexInterrupt()
{
    this->indexDetected = true;
    this->indexCount = this->get();
}

uint32_t hardware_qei::get()
{
    return __HAL_TIM_GET_COUNTER(ptrTimHandler);
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
    ptrTimHandler->Instance = TIM1;

    ptrTimHandler->Init.Prescaler = 0;
    ptrTimHandler->Init.CounterMode = TIM_COUNTERMODE_UP;
    ptrTimHandler->Init.Period = 65535;
    ptrTimHandler->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    ptrTimHandler->Init.RepetitionCounter = 0;
    ptrTimHandler->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
    sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
    sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
    sConfig.IC1Filter = 10;

    sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
    sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
    sConfig.IC2Filter = 10;

    if (HAL_TIM_Encoder_Init(ptrTimHandler, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

    if (HAL_TIMEx_MasterConfigSynchronization(ptrTimHandler, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }


    if (HAL_TIM_Encoder_Start(ptrTimHandler, TIM_CHANNEL_2)!=HAL_OK)
    {
        printf("Couldn't Start Encoder\r\n");
    }
}