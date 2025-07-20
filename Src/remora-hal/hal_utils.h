#ifndef HAL_UTILS_H
#define HAL_UTILS_H

#include "pin/pin.h"
#include "peripheralPins.h"
#include "pinNames.h"
//#include "PinNamesTypes.h" 

SPIName getSPIPeripheralName(PinName, PinName, PinName);
void enableSPIClock(SPI_TypeDef *);

#endif