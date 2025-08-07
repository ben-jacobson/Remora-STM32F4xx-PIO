#ifndef HAL_CONFIGURATION_H
#define HAL_CONFIGURATION_H

#include "stm32f4xx.h"

#include <cstdint>

// these support F401xx, F411xx and F446xx
namespace HAL_Config {
    //__attribute__((section(".json_upload"))) const uint8_t json_upload_area[16384]; // note that these addresses can also be accessed this way, but the code would require significant refactoring to use this so commented out for now. 
    //__attribute__((section(".json_storage"))) const uint8_t json_storage_area[16384];

    constexpr std::uintptr_t JSON_upload_start_address            = 0x08008000; // upload area is second half of storage
    constexpr std::uintptr_t JSON_upload_end_address            = JSON_upload_start_address + (16 * 1024); 

    constexpr std::uintptr_t JSON_storage_start_address           = 0x0800C000;
    constexpr std::uintptr_t JSON_storage_end_address           = JSON_storage_start_address + (16 * 1024);

    constexpr uint32_t JSON_Config_Upload_Sector                          = FLASH_SECTOR_2;  // 16K in flash
    constexpr uint32_t JSON_Config_Storage_Sector                          = FLASH_SECTOR_3;  // 16K in flash
}

#endif