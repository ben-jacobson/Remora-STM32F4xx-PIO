#include "hal_utils.h"

SPIName getSPIPeripheralName(PinName mosi, PinName miso, PinName sclk)        
{
    SPIName spi_mosi = (SPIName)pinmap_peripheral(mosi, PinMap_SPI_MOSI);
    SPIName spi_miso = (SPIName)pinmap_peripheral(miso, PinMap_SPI_MISO);
    SPIName spi_sclk = (SPIName)pinmap_peripheral(sclk, PinMap_SPI_SCLK);

    SPIName spi_per;

    // MISO or MOSI may be not connected
    if (miso == NC) {
        spi_per = (SPIName)pinmap_merge(spi_mosi, spi_sclk);
    } else if (mosi == NC) {
        spi_per = (SPIName)pinmap_merge(spi_miso, spi_sclk);
    } else {
        SPIName spi_data = (SPIName)pinmap_merge(spi_mosi, spi_miso);
        spi_per = (SPIName)pinmap_merge(spi_data, spi_sclk);
    }

    return spi_per;
}

void enableSPIClock(SPI_TypeDef* instance) {
    if (instance == SPI1) __HAL_RCC_SPI1_CLK_ENABLE();
    else if (instance == SPI2) __HAL_RCC_SPI2_CLK_ENABLE();
    else if (instance == SPI3) __HAL_RCC_SPI3_CLK_ENABLE();
    else if (instance == SPI4) __HAL_RCC_SPI4_CLK_ENABLE();
}

Pin* createPin(const std::string& portAndPin, PinName pinName, const PinMap* map, uint32_t gpio_mode, uint32_t gpio_pull, uint32_t gpio_speed) { 
    uint32_t function = STM_PIN_AFNUM(pinmap_function(pinName, map));
    return new Pin(portAndPin, gpio_mode, gpio_pull, gpio_speed, function);
}

void delay_ms(uint32_t ms) {
    HAL_Delay(ms);
}
