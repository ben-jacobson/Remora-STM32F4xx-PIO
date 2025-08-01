#include "STM32F4_EthComms.h"
#include "../remora-core/drivers/W5500_Networking/W5500_Networking.h"

#include "remora-core/configuration.h"  
#ifdef ETH_CTRL

SPI_HandleTypeDef STM32F4_EthComms::spiHandle;
DMA_HandleTypeDef STM32F4_EthComms::hdma_spi_tx;
DMA_HandleTypeDef STM32F4_EthComms::hdma_spi_rx;

STM32F4_EthComms::STM32F4_EthComms(volatile rxData_t* _ptrRxData, volatile txData_t* _ptrTxData, std::string _mosiPortAndPin, std::string _misoPortAndPin, std::string _clkPortAndPin, std::string _csPortAndPin, std::string _rstPortAndPin) :
    mosiPortAndPin(_mosiPortAndPin),
    misoPortAndPin(_misoPortAndPin),
	clkPortAndPin(_clkPortAndPin),
    csPortAndPin(_csPortAndPin),
    rstPortAndPin(_rstPortAndPin)
{
    ptrRxData = _ptrRxData;
	ptrTxData = _ptrTxData;
    //ptrRxDMABuffer = &rxDMABuffer;

    // irqNss = SPI_CS_IRQ;
    // irqDMAtx = DMA1_Stream0_IRQn;
    // irqDMArx = DMA1_Stream1_IRQn;

    mosiPinName = portAndPinToPinName(mosiPortAndPin.c_str());
    misoPinName = portAndPinToPinName(misoPortAndPin.c_str());
    clkPinName = portAndPinToPinName(clkPortAndPin.c_str());
    csPinName = portAndPinToPinName(csPortAndPin.c_str());
    rstPinName = portAndPinToPinName(rstPortAndPin.c_str());
}

STM32F4_EthComms::~STM32F4_EthComms() {

}

void STM32F4_EthComms::init(void) {
    
    printf("EthComms Init\n");
    spiHandle.Instance = (SPI_TypeDef* )getSPIPeripheralName(mosiPinName, misoPinName, clkPinName); // get the SPI handle from the list of GPIO used for Mosi, Miso, SCK

    InitDMAIRQs(spiHandle.Instance);    // set up the IRQs for DMA triggering
    enableSPIClocks(spiHandle.Instance);

    printf("initialising SPI pins\n");
    csPin = new Pin(csPortAndPin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, 0);  
    rstPin = new Pin(rstPortAndPin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, 0);

    // Create alternate function SPI pins
    mosiPin = createPinFromPinMap(mosiPortAndPin, mosiPinName, PinMap_SPI_MOSI);
    misoPin = createPinFromPinMap(misoPortAndPin, misoPinName, PinMap_SPI_MISO);
    clkPin  = createPinFromPinMap(clkPortAndPin,  clkPinName,  PinMap_SPI_SCLK);

    printf("Setting up SPI and DMA\n");

    if (spiHandle.Instance == SPI1)
    {
        printf("Initialising SPI1 DMA\n");
        initDMA(DMA2_Stream0, DMA2_Stream3, DMA_CHANNEL_3);
        HAL_NVIC_SetPriority(SPI1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(SPI1_IRQn);        
    }
    else if (spiHandle.Instance == SPI2)
    {
        printf("Initialising SPI2 DMA\n");
        initDMA(DMA1_Stream3, DMA1_Stream4, DMA_CHANNEL_0);
        HAL_NVIC_SetPriority(SPI2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(SPI2_IRQn);        
    }
    else if (spiHandle.Instance == SPI3)
    {
        printf("Initialising SPI3 DMA\n");
        initDMA(DMA1_Stream0, DMA1_Stream5, DMA_CHANNEL_0);                
        HAL_NVIC_SetPriority(SPI3_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(SPI3_IRQn);        
    }
    else
    {
        printf("Invalid SPI bus selected, please check your SPI configuration in your PlatformIO SPI1-3\n");
        return;
    }    

    // init rest of SPI handle
    spiHandle.Init.Mode           			= SPI_MODE_MASTER;
    spiHandle.Init.Direction      			= SPI_DIRECTION_2LINES;
    spiHandle.Init.DataSize       			= SPI_DATASIZE_8BIT;
    spiHandle.Init.CLKPolarity    			= SPI_POLARITY_LOW;
    spiHandle.Init.CLKPhase       			= SPI_PHASE_1EDGE;
    spiHandle.Init.NSS            			= SPI_NSS_SOFT; 
    spiHandle.Init.BaudRatePrescaler        = SPI_BAUDRATEPRESCALER_2;
    spiHandle.Init.FirstBit       			= SPI_FIRSTBIT_MSB;
    spiHandle.Init.TIMode         			= SPI_TIMODE_DISABLE;
    spiHandle.Init.CRCCalculation 			= SPI_CRCCALCULATION_DISABLE;
    spiHandle.Init.CRCPolynomial  			= 10; 

    if (HAL_SPI_Init(&this->spiHandle) != HAL_OK)
    {
        printf("Error initialising SPI\n");
    }

    // For the time being, the Wiznet code moves the data packet into the buffer. May need to refactor so that it triggers our mem2mem. 
    //printf("Initialising DMA for Memory to Memory transfer\n");

    // hdma_memtomem.Instance 					= DMA1_Stream2;       // F4 doesn't have a nice clean mem2mem so will manually use DMA1
    // hdma_memtomem.Init.Channel 				= DMA_CHANNEL_0;       // aparently not needed for mem2mem but using an unused one anyway. 
    // hdma_memtomem.Init.Direction 			= DMA_MEMORY_TO_MEMORY;
    // hdma_memtomem.Init.PeriphInc 			= DMA_PINC_ENABLE;
    // hdma_memtomem.Init.MemInc 				= DMA_MINC_ENABLE;
    // hdma_memtomem.Init.PeriphDataAlignment 	= DMA_PDATAALIGN_BYTE;
    // hdma_memtomem.Init.MemDataAlignment 	= DMA_MDATAALIGN_BYTE;
    // hdma_memtomem.Init.Mode 				= DMA_NORMAL;
    // hdma_memtomem.Init.Priority 			= DMA_PRIORITY_LOW;
    // hdma_memtomem.Init.FIFOMode 			= DMA_FIFOMODE_ENABLE;
    // hdma_memtomem.Init.FIFOThreshold 		= DMA_FIFO_THRESHOLD_FULL;
    // hdma_memtomem.Init.MemBurst 			= DMA_MBURST_SINGLE;
    // hdma_memtomem.Init.PeriphBurst 			= DMA_PBURST_SINGLE;

    // __HAL_RCC_DMA1_CLK_ENABLE();  
    // HAL_DMA_Init(&hdma_memtomem);

    // // Initialize the data buffers
    // std::fill(std::begin(ptrTxData->txBuffer), std::end(ptrTxData->txBuffer), 0);
    // std::fill(std::begin(ptrRxData->rxBuffer), std::end(ptrRxData->rxBuffer), 0);
    // std::fill(std::begin(ptrRxDMABuffer->buffer[0].rxBuffer), std::end(ptrRxDMABuffer->buffer[0].rxBuffer), 0);
    // std::fill(std::begin(ptrRxDMABuffer->buffer[1].rxBuffer), std::end(ptrRxDMABuffer->buffer[1].rxBuffer), 0);

    // // Start the multi-buffer DMA SPI communication
    // dmaStatus = startMultiBufferDMASPI(
    //     (uint8_t*)ptrTxData->txBuffer,
    //     (uint8_t*)ptrTxData->txBuffer,
    //     (uint8_t*)ptrRxDMABuffer->buffer[0].rxBuffer,
    //     (uint8_t*)ptrRxDMABuffer->buffer[1].rxBuffer,
    //     Config::dataBuffSize
    // );

    // // Check for DMA initialization errors
    // if (dmaStatus != HAL_OK) {
    //     printf("DMA SPI error\n");
    // }
}

void STM32F4_EthComms::start(void) {
    network::EthernetInit(this, csPin, rstPin);
    network::udpServerInit();
    //tftp_handle::IAP_tftpd_init();    
}

void STM32F4_EthComms::tasks(void) {    

    network::EthernetTasks();

    switch (ptrRxData->header)
    {
        case Config::pruRead:
            // No action needed for PRU_READ.
            dataCallback(true);
            break;

        case Config::pruWrite:
            // Valid PRU_WRITE header, flag RX data transfer.
            dataCallback(true);
            break;

        default:
            dataCallback(false);
            break;
    }


    // Data is currently being put directly into buffer, will assess viability of using DMA mem2mem to achieve this. Probably faster. 
	// if (newDataReceived) {
	//     uint8_t* srcBuffer = (uint8_t*)ptrRxDMABuffer->buffer[RXbufferIdx].rxBuffer;
	//     uint8_t* destBuffer = (uint8_t*)ptrRxData->rxBuffer;

	//     __disable_irq();

	//     dmaStatus = HAL_DMA_Start( // old H7 mem to mem code
	//     							&hdma_memtomem,
	// 								(uint32_t)srcBuffer,
	// 								(uint32_t)destBuffer,
	// 								Config::dataBuffSize
	//     							);

	//     if (dmaStatus == HAL_OK) {
	//         dmaStatus = HAL_DMA_PollForTransfer(&hdma_memtomem, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
	//     }

	//     __enable_irq();
	//     HAL_DMA_Abort(&hdma_memtomem);
	// 	newDataReceived = false; // signal we are finished with it
    // }
}

void STM32F4_EthComms::initDMA(DMA_Stream_TypeDef* DMA_RX_Stream, DMA_Stream_TypeDef* DMA_TX_Stream, uint32_t DMA_channel) {
    // RX
    hdma_spi_rx.Instance = DMA_RX_Stream;
    hdma_spi_rx.Init.Channel = DMA_channel; 
    hdma_spi_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_spi_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;    
    hdma_spi_rx.Init.Mode = DMA_NORMAL;
    hdma_spi_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_spi_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    if (HAL_DMA_Init(&hdma_spi_rx) != HAL_OK)
    {
        Error_Handler();
    }

    __HAL_LINKDMA(&spiHandle, hdmarx, hdma_spi_rx);

    // // TX
    hdma_spi_tx.Instance = DMA_TX_Stream;
    hdma_spi_tx.Init.Channel = DMA_channel;
    hdma_spi_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi_tx.Init.Mode = DMA_NORMAL;
    hdma_spi_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_spi_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    if (HAL_DMA_Init(&hdma_spi_tx) != HAL_OK)
    {
        Error_Handler();
    }

    __HAL_LINKDMA(&spiHandle, hdmatx, hdma_spi_tx);
}

uint8_t STM32F4_EthComms::read_byte(void)
{
	spiHandle.Instance->DR = 0xFF; // Writing dummy data into Data register

    while(!__HAL_SPI_GET_FLAG(&spiHandle, SPI_FLAG_RXNE));

    return (uint8_t)spiHandle.Instance->DR;
}

uint8_t STM32F4_EthComms::write_byte(uint8_t byte)
{
	spiHandle.Instance->DR = byte;

    while(!__HAL_SPI_GET_FLAG(&spiHandle, SPI_FLAG_TXE));
    while(!__HAL_SPI_GET_FLAG(&spiHandle, SPI_FLAG_RXNE));

    __HAL_SPI_CLEAR_OVRFLAG(&spiHandle);

    return (uint8_t)spiHandle.Instance->DR;
}

void STM32F4_EthComms::DMA_write(uint8_t *data, uint16_t len)
{
    if(HAL_SPI_Transmit_DMA(&spiHandle, data, len) == HAL_OK)
        while(spiHandle.State != HAL_SPI_STATE_READY);
}

void STM32F4_EthComms::DMA_read(uint8_t *data, uint16_t len)
{
    if (HAL_SPI_Receive_DMA(&spiHandle, data, len) == HAL_OK)
        while(spiHandle.State != HAL_SPI_STATE_READY);
}

#endif