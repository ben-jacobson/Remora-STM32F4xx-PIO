#ifndef QEIDRIVER_H
#define QEIDRIVER_H

#include "stm32f4xx_hal.h"
#include "../../remora-core/configuration.h"
#include "../../remora-core/modules/moduleInterrupt.h"
#include "../hal_utils.h"
#include "../shared_handlers.h"

class Hardware_QEI
{
    private:
        TIM_HandleTypeDef*              ptrTimHandler = nullptr;

        IRQn_Type 		                irqIndex;       
    	ModuleInterrupt<Hardware_QEI>*	IndexInterrupt;

        std::string                     indexPortAndPin;
        std::string                     chAPortAndPin;
        std::string                     chBPortAndPin;
        Pin*                            indexPin;
        Pin*                            chAPin;
        Pin*                            chBPin;

        bool                            hasIndex;
        int                             modifier; 

        void handleIndexInterrupt(void);

    public:
        bool                            indexDetected;
        int32_t                         indexCount;

        Hardware_QEI(bool _hasIndex, int _modifier); 
        void init(void);
        uint32_t get(void);
};

#endif
