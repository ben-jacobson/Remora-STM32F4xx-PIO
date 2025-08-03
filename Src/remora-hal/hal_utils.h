#ifndef HAL_UTILS_H
#define HAL_UTILS_H

#include "main.h"
#include "pin/pin.h"
#include "peripheralPins.h"
#include "pinNames.h"
#include "PinNamesTypes.h" 

// Macro's for Remora Core, these map 1:1 in STM32, but in other platforms will require custom  calls to replace functionality. 
#define lock_flash          HAL_FLASH_Lock
#define unlock_flash        HAL_FLASH_Lock
#define write_to_flash      HAL_FLASH_Program
#define mass_erase_flash    HAL_FLASHEx_Erase


SPIName getSPIPeripheralName(PinName, PinName, PinName);
void enableSPIClocks(SPI_TypeDef *);
void InitDMAIRQs(SPI_TypeDef *);

Pin* createPinFromPinMap(const std::string&, PinName, const PinMap*,
               uint32_t = GPIO_MODE_AF_PP,
               uint32_t = GPIO_NOPULL,
               uint32_t = GPIO_SPEED_FREQ_VERY_HIGH);

void delay_ms(uint32_t);

void mass_erase_flash_sector(uint32_t);

#endif