#ifndef HAL_CONFIGURATION_H
#define HAL_CONFIGURATION_H

#include "stm32f4xx.h"

#include <cstdint>

namespace HAL_Config {
    constexpr std::uintptr_t JSON_storage_address           = 0x8060000;
    constexpr std::uintptr_t JSON_upload_address            = JSON_storage_address + (64 * 1024);   // Uses last half of sector. Pls keep your config.txt files below 64Kb.
    constexpr std::uintptr_t user_flash_last_page_address   = JSON_storage_address;
    constexpr std::uintptr_t user_flash_end_address         = JSON_storage_address + (128 * 1024) - 1;
    constexpr uint32_t JSON_SECTOR                          = FLASH_SECTOR_7;
}

#endif