#include "STM32F4_EthComms.h"

STM32F4_EthComms::STM32F4_EthComms(volatile rxData_t* _ptrRxData, volatile txData_t* _ptrTxData, std::string _mosiPortAndPin, std::string _misoPortAndPin, std::string _clkPortAndPin, std::string _csPortAndPin) :
	ptrRxData(_ptrRxData),
	ptrTxData(_ptrTxData),
    mosiPortAndPin(_mosiPortAndPin),
    misoPortAndPin(_misoPortAndPin),
	clkPortAndPin(_clkPortAndPin),
    csPortAndPin(_csPortAndPin)
{
    //ptrRxDMABuffer = &rxDMABuffer;

    // irqNss = SPI_CS_IRQ;
    // irqDMAtx = DMA1_Stream0_IRQn;
    // irqDMArx = DMA1_Stream1_IRQn;

    mosiPinName = portAndPinToPinName(mosiPortAndPin.c_str());
    misoPinName = portAndPinToPinName(misoPortAndPin.c_str());
    clkPinName = portAndPinToPinName(clkPortAndPin.c_str());
    csPinName = portAndPinToPinName(csPortAndPin.c_str());

    spiHandle.Instance = (SPI_TypeDef* )getSPIPeripheralName(mosiPinName, misoPinName, clkPinName);
}

STM32F4_EthComms::~STM32F4_EthComms() {

}

SPIName STM32F4_EthComms::getSPIPeripheralName(PinName mosi, PinName miso, PinName sclk)        // Todo - this is exactly the same in both comms classes, should we make it into it's own separate function? Could be better as a generic utility?
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

void STM32F4_EthComms::init(void) {
    // remora Class constructor calls init and start one after the other

    // Set up SPI bus (with single NSS?)

    // Set up interrupt on specific pin other than NSS
}

void STM32F4_EthComms::start(void) {
    // remora Class constructor calls init and start one after the other

    // Set up DMA to enable mem2mem between slave device (WizNET) back to master (MCU)

    // Set up interrupt to set new data flag
}

void STM32F4_EthComms::tasks(void) {    
    // Remora run will call tasks

	if (newDataReceived) {
	    // uint8_t* srcBuffer = (uint8_t*)ptrRxDMABuffer->buffer[RXbufferIdx].rxBuffer;
	    // uint8_t* destBuffer = (uint8_t*)ptrRxData->rxBuffer;

	    // __disable_irq();

	    // dmaStatus = HAL_DMA_Start( // old H7 mem to mem code
	    // 							&hdma_memtomem,
		// 							(uint32_t)srcBuffer,
		// 							(uint32_t)destBuffer,
		// 							Config::dataBuffSize
	    // 							);

	    // if (dmaStatus == HAL_OK) {
	    //     dmaStatus = HAL_DMA_PollForTransfer(&hdma_memtomem, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
	    // }

	    // __enable_irq();
	    // HAL_DMA_Abort(&hdma_memtomem);
		newDataReceived = false; // signal we are finished with it
    }
}