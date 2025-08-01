#ifndef HAL_UTILS_H
#define HAL_UTILS_H

#include "pin/pin.h"
#include "peripheralPins.h"
#include "pinNames.h"
#include "PinNamesTypes.h" 

SPIName getSPIPeripheralName(PinName, PinName, PinName);
void enableSPIClocks(SPI_TypeDef *);
void InitDMAIRQs(SPI_TypeDef *);

Pin* createPinFromPinMap(const std::string&, PinName, const PinMap*,
               uint32_t = GPIO_MODE_AF_PP,
               uint32_t = GPIO_NOPULL,
               uint32_t = GPIO_SPEED_FREQ_VERY_HIGH);

void delay_ms(uint32_t);

#endif