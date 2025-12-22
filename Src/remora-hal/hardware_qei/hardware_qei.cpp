#include "hardware_qei.h"

// hardware_qei::hardware_qei() :
//     qeiIndex(NC)
// {
//     this->hasIndex = false;
//     this->init();
// }


// hardware_qei::hardware_qei(bool hasIndex) :
//     hasIndex(hasIndex),
//     qeiIndex(PE_13)
// {
//     this->hasIndex = true;
//     this->irq = EXTI15_10_IRQn;

//     this->init();

//     qeiIndex.rise(callback(this, &hardware_qei::interruptHandler));
//     //NVIC_EnableIRQ(this->irq);
//     HAL_NVIC_SetPriority(this->irq, 0, 0);
// }


// void hardware_qei::interruptHandler()
// {
//     this->indexDetected = true;
//     this->indexCount = this->get();
// }


// uint32_t hardware_qei::get()
// {
//     return __HAL_TIM_GET_COUNTER(&htim);
// }

// void hardware_qei::init()
// {
//     printf("  Initialising hardware QEI module\n");

//     this->htim.Instance = TIM1;
//     this->htim.Init.Prescaler = 0;
//     this->htim.Init.CounterMode = TIM_COUNTERMODE_UP;
//     this->htim.Init.Period = 65535;
//     //this->htim.Init.Period = 0xffffffff; // 32-bit count for TIM2
//     this->htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
//     this->htim.Init.RepetitionCounter = 0;

//     this->sConfig.EncoderMode = TIM_ENCODERMODE_TI12;

//     this->sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
//     this->sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
//     this->sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
//     this->sConfig.IC1Filter = 0;

//     this->sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
//     this->sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
//     this->sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
//     this->sConfig.IC2Filter = 0;

//     if (HAL_TIM_Encoder_Init(&this->htim, &this->sConfig) != HAL_OK)
//     {
//         printf("Couldn't Init Encoder\r\n");
//     }

//     this->sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
//     this->sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
//     HAL_TIMEx_MasterConfigSynchronization(&this->htim, &this->sMasterConfig);

//     if (HAL_TIM_Encoder_Start(&this->htim, TIM_CHANNEL_2)!=HAL_OK)
//     {
//         printf("Couldn't Start Encoder\r\n");
//     }
// }


// void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef* htim_encoder)
// {
//     GPIO_InitTypeDef GPIO_InitStruct = {0};
//     if(htim_encoder->Instance==TIM1)
//     {
//         __HAL_RCC_TIM1_CLK_ENABLE();

//         __HAL_RCC_GPIOE_CLK_ENABLE();
//         /**TIM1 GPIO Configuration
//         PE9     ------> TIM1_CH1
//         PE11     ------> TIM1_CH2
//         */
//         GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_11;
//         GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//         GPIO_InitStruct.Pull = GPIO_NOPULL;
//         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//         GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
//         HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
//     }
// }
