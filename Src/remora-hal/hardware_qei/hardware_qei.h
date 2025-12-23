#ifndef QEIDRIVER_H
#define QEIDRIVER_H

#include "stm32f4xx_hal.h"
#include "../../remora-core/modules/moduleInterrupt.h"
#include "../hal_utils.h"
#include "../shared_handlers.h"

class hardware_qei
{
    private:
        TIM_HandleTypeDef*              ptrTimHandler = nullptr;

        IRQn_Type 		                irqIndex;       
    	ModuleInterrupt<hardware_qei>*	IndexInterrupt;

        std::string                     indexPortAndPin;
        PinName                         indexPinName;
        Pin*                            indexPin;
        
        bool                            hasIndex;
        bool                            indexDetected;
        int32_t                         indexCount;

        void handleIndexInterrupt(void);

    public:
        hardware_qei(bool _hasIndex); 
        void init(void);
        uint32_t get(void);
};

#endif
