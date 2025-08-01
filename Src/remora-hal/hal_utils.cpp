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

void enableSPIClocks(SPI_TypeDef* spi_instance) {
    if (spi_instance == SPI1) __HAL_RCC_SPI1_CLK_ENABLE();
    else if (spi_instance == SPI2) __HAL_RCC_SPI2_CLK_ENABLE();
    else if (spi_instance == SPI3) __HAL_RCC_SPI3_CLK_ENABLE();
    else if (spi_instance == SPI4) __HAL_RCC_SPI4_CLK_ENABLE();
}

void InitDMAIRQs(SPI_TypeDef* spi_instance)
{
    if (spi_instance == SPI1)
    {
        __HAL_RCC_DMA2_CLK_ENABLE();

        HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
        HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);              
    }
    else if (spi_instance == SPI2)
    {
        __HAL_RCC_DMA1_CLK_ENABLE();

        HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
        HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);       
    }
    else if (spi_instance == SPI3)
    {
        __HAL_RCC_DMA1_CLK_ENABLE();

        HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
        HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);     
    }
    else
    {
        printf("Invalid SPI bus selected, please check your SPI configuration in your PlatformIO SPI1-3\n");
        return;
    }
}

Pin* createPinFromPinMap(const std::string& portAndPin, PinName pinName, const PinMap* map, uint32_t gpio_mode, uint32_t gpio_pull, uint32_t gpio_speed) { 
    uint32_t function = STM_PIN_AFNUM(pinmap_function(pinName, map));
    return new Pin(portAndPin, gpio_mode, gpio_pull, gpio_speed, function);
}

void delay_ms(uint32_t ms) {
    HAL_Delay(ms);
}