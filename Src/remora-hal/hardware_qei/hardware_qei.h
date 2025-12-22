#ifndef QEIDRIVER_H
#define QEIDRIVER_H

#include "stm32f4xx_hal.h"
#include "../hal_utils.h"
#include "../shared_handlers.h"

class hardware_qei
{
    private:
        TIM_HandleTypeDef* ptr_tim_handler = nullptr;

        IRQn_Type 		        irqIndex;
        Pin*                    indexPin;

        bool                    hasIndex;
        bool                    indexDetected;
        int32_t                 indexCount;

        void interruptHandler();

    public:
        hardware_qei();            // for channel A & B
        hardware_qei(bool hasIndex);        // For channels A & B, and index

        void init(void);
        uint32_t get(void);
};

#endif
