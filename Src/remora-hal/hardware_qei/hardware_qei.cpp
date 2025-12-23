#include "hardware_qei.h"

Hardware_QEI::Hardware_QEI(bool _hasIndex, int _modifier) :
    hasIndex(_hasIndex),
    modifier(_modifier)
{
    this->init();

    if (hasIndex) 
    {
        indexPortAndPin = "PC_7";
        indexPin = new Pin(indexPortAndPin, GPIO_MODE_IT_RISING, modifier, GPIO_SPEED_FREQ_HIGH, 0);  

        irqIndex = EXTI9_5_IRQn;

        IndexInterrupt = new ModuleInterrupt<Hardware_QEI>
        (
            irqIndex,
            this,
            &Hardware_QEI::handleIndexInterrupt
        );        

        HAL_NVIC_SetPriority(this->irqIndex, Config::qeiIndexPriority, 0); 
        HAL_NVIC_EnableIRQ(this->irqIndex);        
    }
}

void Hardware_QEI::handleIndexInterrupt()
{
    this->indexDetected = true;
    this->indexCount = this->get();
}

uint32_t Hardware_QEI::get()
{
    return __HAL_TIM_GET_COUNTER(ptrTimHandler);
}

void Hardware_QEI::init()
{
    printf("  Initialising hardware QEI module\n");

    __HAL_RCC_TIM1_CLK_ENABLE();

    chAPortAndPin = "PA_8";
    chBPortAndPin = "PA_9";

    chAPin = new Pin(chAPortAndPin, GPIO_MODE_AF_PP, modifier, GPIO_SPEED_FREQ_HIGH, GPIO_AF1_TIM1);
    chBPin = new Pin(chBPortAndPin, GPIO_MODE_AF_PP, modifier, GPIO_SPEED_FREQ_HIGH, GPIO_AF1_TIM1);

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