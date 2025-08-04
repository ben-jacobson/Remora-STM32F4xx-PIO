#ifndef HAL_CONFIGURATION_H
#define HAL_CONFIGURATION_H

#include "stm32f4xx.h"

#include <cstdint>

// these support F401xx, F411xx and F446xx
namespace HAL_Config {
    constexpr std::uintptr_t JSON_storage_address           = 0x8060000;
    constexpr std::uintptr_t JSON_upload_address            = 0x8040000;   
    constexpr std::uintptr_t user_flash_last_page_address   = JSON_storage_address;
    constexpr std::uintptr_t user_flash_end_address         = JSON_storage_address + (128 * 1024) - 1;
    constexpr uint32_t JSON_Sector                          = FLASH_SECTOR_7;
}

#endif