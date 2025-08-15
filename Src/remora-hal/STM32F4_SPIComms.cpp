#include "STM32F4_SPIComms.h"

volatile DMA_RxBuffer_t rxDMABuffer;

static void MultiBufferDMAErrorCallback(DMA_HandleTypeDef *hdma);

STM32F4_SPIComms::STM32F4_SPIComms(volatile rxData_t* _ptrRxData, volatile txData_t* _ptrTxData, std::string _mosiPortAndPin, std::string _misoPortAndPin, std::string _clkPortAndPin, std::string _csPortAndPin) :
	ptrRxData(_ptrRxData),
	ptrTxData(_ptrTxData),
    mosiPortAndPin(_mosiPortAndPin),
    misoPortAndPin(_misoPortAndPin),
	clkPortAndPin(_clkPortAndPin),
    csPortAndPin(_csPortAndPin)
{
    mosiPinName = portAndPinToPinName(mosiPortAndPin.c_str());
    misoPinName = portAndPinToPinName(misoPortAndPin.c_str());
    clkPinName = portAndPinToPinName(clkPortAndPin.c_str());
    csPinName = portAndPinToPinName(csPortAndPin.c_str());

    ptrRxDMABuffer = &rxDMABuffer;
}

STM32F4_SPIComms::~STM32F4_SPIComms() {
}

void STM32F4_SPIComms::init() {
    printf("SPIComms Init\n");

    spiHandle.Instance = (SPI_TypeDef* )getSPIPeripheralName(mosiPinName, misoPinName, clkPinName);

    InitDMAIRQs(spiHandle.Instance);    // set up the IRQs for DMA triggering
    enableSPIClocks(spiHandle.Instance);

    // Configure the NSS (chip select) pin as interrupt
    csPin = new Pin(csPortAndPin, GPIO_MODE_IT_RISING, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0);

    printf("initialising SPI pins\n");
    // Create alternate function SPI pins
    mosiPin = createPinFromPinMap(mosiPortAndPin, mosiPinName, PinMap_SPI_MOSI);
    misoPin = createPinFromPinMap(misoPortAndPin, misoPinName, PinMap_SPI_MISO);
    clkPin  = createPinFromPinMap(clkPortAndPin,  clkPinName,  PinMap_SPI_SCLK);
    csPin   = createPinFromPinMap(csPortAndPin,   csPinName,   PinMap_SPI_SSEL);

    printf("Setting up SPI and DMA\n");

    irqNss = SPI_CS_IRQ;

    if (spiHandle.Instance == SPI1)
    {
        printf("Initialising SPI1 DMA\n");
        initSPIDMA(DMA2_Stream0, DMA2_Stream3, DMA_CHANNEL_3);
        irqDMArx = DMA2_Stream0_IRQn;
        irqDMAtx = DMA2_Stream3_IRQn;            
    }
    else if (spiHandle.Instance == SPI2)
    {
        printf("Initialising SPI2 DMA\n");   
        initSPIDMA(DMA1_Stream3, DMA1_Stream4, DMA_CHANNEL_0);
        irqDMArx = DMA1_Stream3_IRQn;
        irqDMAtx = DMA1_Stream4_IRQn;             
    }
    else if (spiHandle.Instance == SPI3)
    {
        printf("Initialising SPI3 DMA\n");
        initSPIDMA(DMA1_Stream0, DMA1_Stream5, DMA_CHANNEL_0);
        irqDMArx = DMA1_Stream0_IRQn;
        irqDMAtx = DMA1_Stream5_IRQn;            
    }
    else
    {
        printf("Unknown SPI instance, please configure in your PlatformIO SPI1-3\n");
        return;
    }

    // Register the NSS (slave select) interrupt
    NssInterrupt = new ModuleInterrupt<STM32F4_SPIComms>(
        irqNss,
        this,
        &STM32F4_SPIComms::handleNssInterrupt
    );
    HAL_NVIC_SetPriority(irqNss, Config::spiNssIrqPriority, 0);
    HAL_NVIC_EnableIRQ(irqNss);

    // Register the DMA Rx interrupt
    dmaRxInterrupt = new ModuleInterrupt<STM32F4_SPIComms>(
        irqDMArx,
        this,
        &STM32F4_SPIComms::handleRxInterrupt
    );
    HAL_NVIC_SetPriority(irqDMArx, Config::spiDmaRxIrqPriority, 0);
    HAL_NVIC_EnableIRQ(irqDMArx);

    // Register the DMA Tx interrupt
    dmaTxInterrupt = new ModuleInterrupt<STM32F4_SPIComms>(
        irqDMAtx,
        this,
        &STM32F4_SPIComms::handleTxInterrupt
    );
    HAL_NVIC_SetPriority(irqDMAtx, Config::spiDmaTxIrqPriority, 0); // TX needs higher priority than RX
    HAL_NVIC_EnableIRQ(irqDMAtx);

    spiHandle.Init.Mode           			= SPI_MODE_SLAVE;
    spiHandle.Init.Direction      			= SPI_DIRECTION_2LINES;
    spiHandle.Init.DataSize       			= SPI_DATASIZE_8BIT;
    spiHandle.Init.CLKPolarity    			= SPI_POLARITY_LOW;
    spiHandle.Init.CLKPhase       			= SPI_PHASE_1EDGE;
    spiHandle.Init.NSS            			= SPI_NSS_HARD_INPUT; // Or SPI_NSS_SOFT ??
    spiHandle.Init.BaudRatePrescaler        = SPI_BAUDRATEPRESCALER_2;
    spiHandle.Init.FirstBit       			= SPI_FIRSTBIT_MSB;
    spiHandle.Init.TIMode         			= SPI_TIMODE_DISABLE;
    spiHandle.Init.CRCCalculation 			= SPI_CRCCALCULATION_DISABLE;
    spiHandle.Init.CRCPolynomial  			= 10; // was 0x0;
    
    HAL_SPI_Init(&this->spiHandle);
    if (HAL_SPI_Init(&this->spiHandle) != HAL_OK) 
    {
        printf("Error initialising SPI\n");
    }

    printf("Initialising DMA for Memory to Memory transfer\n");

    /* TODO!!
        Need to wire up an IRQ response for the mem 2 mem
    */

    hdma_memtomem.Instance 					= DMA1_Stream2;       // F4 doesn't have a nice clean mem2mem so will manually use DMA1
    hdma_memtomem.Init.Channel 				= DMA_CHANNEL_0;       // aparently not needed for mem2mem but using an unused one anyway. 
    hdma_memtomem.Init.Direction 			= DMA_MEMORY_TO_MEMORY;
    hdma_memtomem.Init.PeriphInc 			= DMA_PINC_ENABLE;
    hdma_memtomem.Init.MemInc 				= DMA_MINC_ENABLE;
    hdma_memtomem.Init.PeriphDataAlignment 	= DMA_PDATAALIGN_BYTE;
    hdma_memtomem.Init.MemDataAlignment 	= DMA_MDATAALIGN_BYTE;
    hdma_memtomem.Init.Mode 				= DMA_NORMAL;
    hdma_memtomem.Init.Priority 			= DMA_PRIORITY_LOW;
    hdma_memtomem.Init.FIFOMode 			= DMA_FIFOMODE_ENABLE;
    hdma_memtomem.Init.FIFOThreshold 		= DMA_FIFO_THRESHOLD_FULL;
    hdma_memtomem.Init.MemBurst 			= DMA_MBURST_SINGLE;
    hdma_memtomem.Init.PeriphBurst 			= DMA_PBURST_SINGLE;

    __HAL_RCC_DMA1_CLK_ENABLE();  
    if (HAL_DMA_Init(&hdma_memtomem) != HAL_OK) 
    {
        printf("Error initialising SPI mem to mem\n");
    }
}

void STM32F4_SPIComms::initSPIDMA(DMA_Stream_TypeDef* DMA_RX_Stream, DMA_Stream_TypeDef* DMA_TX_Stream, uint32_t DMA_channel) {
    // RX
    hdma_spi_rx.Instance = DMA_RX_Stream;
    hdma_spi_rx.Init.Channel = DMA_channel; 
    hdma_spi_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_spi_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi_rx.Init.Mode = DMA_CIRCULAR;
    hdma_spi_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_spi_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    if (HAL_DMA_Init(&hdma_spi_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(&spiHandle, hdmarx, hdma_spi_rx);

    // TX
    hdma_spi_tx.Instance = DMA_TX_Stream;
    hdma_spi_tx.Init.Channel = DMA_channel;
    hdma_spi_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi_tx.Init.Mode = DMA_CIRCULAR;
    hdma_spi_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_spi_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    if (HAL_DMA_Init(&hdma_spi_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(&spiHandle, hdmatx, hdma_spi_tx);
}

void STM32F4_SPIComms::start() {

    // Initialize the data buffers
    std::fill(std::begin(ptrTxData->txBuffer), std::end(ptrTxData->txBuffer), 0);
    std::fill(std::begin(ptrRxData->rxBuffer), std::end(ptrRxData->rxBuffer), 0);
    std::fill(std::begin(ptrRxDMABuffer->buffer[0].rxBuffer), std::end(ptrRxDMABuffer->buffer[0].rxBuffer), 0);
    std::fill(std::begin(ptrRxDMABuffer->buffer[1].rxBuffer), std::end(ptrRxDMABuffer->buffer[1].rxBuffer), 0);

    // Start the multi-buffer DMA SPI communication
    dmaStatus = startMultiBufferDMASPI(
        (uint8_t*)ptrTxData->txBuffer,
        (uint8_t*)ptrTxData->txBuffer,
        (uint8_t*)ptrRxDMABuffer->buffer[0].rxBuffer,
        (uint8_t*)ptrRxDMABuffer->buffer[1].rxBuffer,
        Config::dataBuffSize
    );

    // Check for DMA initialization errors
    if (dmaStatus != HAL_OK) {
        printf("DMA SPI error\n");
    }
}

HAL_StatusTypeDef STM32F4_SPIComms::startMultiBufferDMASPI(uint8_t *pTxBuffer0, uint8_t *pTxBuffer1,
                                                   uint8_t *pRxBuffer0, uint8_t *pRxBuffer1,
                                                   uint16_t Size)
{
    // Check Direction parameter 
    assert_param(IS_SPI_DIRECTION_2LINES(spiHandle.Init.Direction));

    if (spiHandle.State != HAL_SPI_STATE_READY)
    {
        return HAL_BUSY;
    }

    if ((pTxBuffer0 == NULL) || (pRxBuffer0 == NULL) || (Size == 0UL))
    {
        return HAL_ERROR;
    }

    // If secondary Tx or Rx buffer is not provided, use the primary buffer 
    if (pTxBuffer1 == NULL)
    {
        pTxBuffer1 = pTxBuffer0;
    }

    if (pRxBuffer1 == NULL)
    {
        pRxBuffer1 = pRxBuffer0;
    }

    // Lock the process 
    __HAL_LOCK(&spiHandle);

    // Set the transaction information 
    spiHandle.State       = HAL_SPI_STATE_BUSY_TX_RX;
    spiHandle.ErrorCode   = HAL_SPI_ERROR_NONE;
    spiHandle.TxXferSize  = Size;
    spiHandle.TxXferCount = Size;
    spiHandle.RxXferSize  = Size;
    spiHandle.RxXferCount = Size;

    // Init unused fields in handle to zero 
    spiHandle.RxISR       = NULL;
    spiHandle.TxISR       = NULL;

    /* 
    
    Debugging note for later!!
    
    Porting this over from an H7 was problematic, the internal register map is quite different
    Have made the best effort possible to map these back to F4 compatibility but cannot be sure if it worked until something breaks far down the track,
    This could be a good place to start if anything starts going wrong with the Ethernet data packet DMA transfers. 
    
    */


    // Set Full-Duplex mode 
    // SPI_2LINES(&spiHandle); // we have ensured 2lines is configured in init()

    // Reset the Tx/Rx DMA bits 
    CLEAR_BIT(spiHandle.Instance->CR2, SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN); //    was  CLEAR_BIT(spiHandle.Instance->CFG1, SPI_CFG1_TXDMAEN | SPI_CFG1_RXDMAEN);

    // Adjust XferCount according to DMA alignment / Data size 
    if (spiHandle.Init.DataSize <= SPI_DATASIZE_8BIT)
    {
        if (hdma_spi_tx.Init.MemDataAlignment == DMA_MDATAALIGN_HALFWORD)
        {
            spiHandle.TxXferCount = (spiHandle.TxXferCount + 1UL) >> 1UL;
        }
        if (hdma_spi_rx.Init.MemDataAlignment == DMA_MDATAALIGN_HALFWORD)
        {
            spiHandle.RxXferCount = (spiHandle.RxXferCount + 1UL) >> 1UL;
        }
    }
    else if (spiHandle.Init.DataSize <= SPI_DATASIZE_16BIT)
    {
        if (hdma_spi_tx.Init.MemDataAlignment == DMA_MDATAALIGN_WORD)
        {
            spiHandle.TxXferCount = (spiHandle.TxXferCount + 1UL) >> 1UL;
        }
        if (hdma_spi_rx.Init.MemDataAlignment == DMA_MDATAALIGN_WORD)
        {
            spiHandle.RxXferCount = (spiHandle.RxXferCount + 1UL) >> 1UL;
        }
    }

    // Configure Tx DMA with Multi-Buffer 
    hdma_spi_tx.XferHalfCpltCallback = [](DMA_HandleTypeDef *hdma) {};  // The code this was ported from had these set to Nulls, however the F4 implementation of HAL_DMAEx_MultiBufferStart_IT errors out on that, 
    hdma_spi_tx.XferCpltCallback     = [](DMA_HandleTypeDef *hdma) {};  // seeing as there wasn't any intention to do anything, have swapped these for some anonymous functions. 
    hdma_spi_tx.XferErrorCallback    = MultiBufferDMAErrorCallback;

    if (HAL_DMAEx_MultiBufferStart_IT(&hdma_spi_tx,
                                    (uint32_t)pTxBuffer0,
                                    (uint32_t)&spiHandle.Instance->DR,    // RXDR isn't avaialable on F4
                                    (uint32_t)pTxBuffer1,
                                    spiHandle.TxXferCount) != HAL_OK)
    {
        __HAL_UNLOCK(&spiHandle);
        return HAL_ERROR;
    }

    // Configure Rx DMA with Multi-Buffer 
    hdma_spi_rx.XferHalfCpltCallback = [](DMA_HandleTypeDef *hdma) {}; 
    hdma_spi_rx.XferCpltCallback     = [](DMA_HandleTypeDef *hdma) {}; 
    hdma_spi_rx.XferErrorCallback    = MultiBufferDMAErrorCallback; 

    if (HAL_OK != HAL_DMAEx_MultiBufferStart_IT(&hdma_spi_rx,
                                                (uint32_t)&spiHandle.Instance->DR,    // RXDR isn't avaialable on F4
                                                (uint32_t)pRxBuffer0,
                                                (uint32_t)pRxBuffer1,
                                                spiHandle.RxXferCount))
    {
        (void)HAL_DMA_Abort(&hdma_spi_tx);
        __HAL_UNLOCK(&spiHandle);
        return HAL_ERROR;
    }

    // Configure SPI TSIZE for full transfer or circular mode 
    /*if (hdma_spi_rx.Init.Mode == DMA_CIRCULAR || hdma_spi_tx.Init.Mode == DMA_CIRCULAR) // disabled this for now, have ensured circular mode is set up in init()
    {
        MODIFY_REG(spiHandle.Instance->CR2, SPI_CR2_TSIZE, 0UL);
    }
    else
    {
        MODIFY_REG(spiHandle.Instance->CR2, SPI_CR2_TSIZE, Size);
    }*/

    // Enable Tx and Rx DMA Requests 
    SET_BIT(spiHandle.Instance->CR2, SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN); // was SET_BIT(spiHandle.Instance->CFG1, SPI_CFG1_TXDMAEN | SPI_CFG1_RXDMAEN);


    // Enable SPI error interrupt 
    __HAL_SPI_ENABLE_IT(&spiHandle, SPI_IT_ERR); // was  __HAL_SPI_ENABLE_IT(&spiHandle, (SPI_IT_OVR | SPI_IT_UDR | SPI_IT_FRE | SPI_IT_MODF));


    // Enable SPI peripheral 
    __HAL_SPI_ENABLE(&spiHandle);

    if (spiHandle.Init.Mode == SPI_MODE_MASTER)
    {
        //SET_BIT(spiHandle.Instance->CR1, SPI_CR1_CSTART); // nothing to replace it with, SPI transfer is implicitly started by writing to DR above.
    }

    __HAL_UNLOCK(&spiHandle);

    return HAL_OK;
}

int STM32F4_SPIComms::handleDMAInterrupt(DMA_HandleTypeDef *hdma)
{
  // this seems to be a refactor of void HAL_DMA_IRQHandler(DMA_HandleTypeDef *hdma)? I wonder if this could be replaced with STM HAL??

  uint32_t tmpisr_dma;
  int interrupt;

  // calculate DMA base and stream number 
  DMA_Base_Registers  *regs_dma  = (DMA_Base_Registers *)hdma->StreamBaseAddress;

  tmpisr_dma  = regs_dma->ISR;

  //if(IS_DMA_STREAM_INSTANCE(hdma->Instance) != 0U)  // DMA1 or DMA2 instance  - we don't have that macro in F4
    if ((hdma->Instance == DMA2_Stream3) ||  // SPI1 TX (DMA2 Stream3) // need to check that DMA and stream used for all SPI1-3
        (hdma->Instance == DMA2_Stream0) ||  // SPI1 RX (DMA2 Stream0)
        (hdma->Instance == DMA1_Stream4) ||  // SPI2 TX (DMA1 Stream4)
        (hdma->Instance == DMA1_Stream3) ||  // SPI2 RX (DMA1 Stream3)
        (hdma->Instance == DMA1_Stream5) ||  // SPI3 TX (DMA1 Stream5)
        (hdma->Instance == DMA1_Stream0))    // SPI3 RX (DMA1 Stream0)  // doesn't seem that we use this for mem2mem, so worth looking at later.
    {
    // Transfer Error Interrupt management **************************************
    if ((tmpisr_dma & (DMA_FLAG_TEIF0_4 << (hdma->StreamIndex & 0x1FU))) != 0U)
    {
      if(__HAL_DMA_GET_IT_SOURCE(hdma, DMA_IT_TE) != 0U)
      {
        // Disable the transfer error interrupt 
        ((DMA_Stream_TypeDef   *)hdma->Instance)->CR  &= ~(DMA_IT_TE);

        // Clear the transfer error flag 
        regs_dma->IFCR = DMA_FLAG_TEIF0_4 << (hdma->StreamIndex & 0x1FU);

        // Update error code 
        hdma->ErrorCode |= HAL_DMA_ERROR_TE;
        interrupt =  DMA_OTHER;
      }
    }
    // FIFO Error Interrupt management *****************************************
    if ((tmpisr_dma & (DMA_FLAG_FEIF0_4 << (hdma->StreamIndex & 0x1FU))) != 0U)
    {
      if(__HAL_DMA_GET_IT_SOURCE(hdma, DMA_IT_FE) != 0U)
      {
        // Clear the FIFO error flag 
        regs_dma->IFCR = DMA_FLAG_FEIF0_4 << (hdma->StreamIndex & 0x1FU);

        // Update error code 
        hdma->ErrorCode |= HAL_DMA_ERROR_FE;
        interrupt =  DMA_OTHER;
      }
    }
    // Direct Mode Error Interrupt management **********************************
    if ((tmpisr_dma & (DMA_FLAG_DMEIF0_4 << (hdma->StreamIndex & 0x1FU))) != 0U)
    {
      if(__HAL_DMA_GET_IT_SOURCE(hdma, DMA_IT_DME) != 0U)
      {
        // Clear the direct mode error flag 
        regs_dma->IFCR = DMA_FLAG_DMEIF0_4 << (hdma->StreamIndex & 0x1FU);

        // Update error code 
        hdma->ErrorCode |= HAL_DMA_ERROR_DME;
        interrupt =  DMA_OTHER;
      }
    }
    // Half Transfer Complete Interrupt management *****************************
    if ((tmpisr_dma & (DMA_FLAG_HTIF0_4 << (hdma->StreamIndex & 0x1FU))) != 0U)
    {
      if(__HAL_DMA_GET_IT_SOURCE(hdma, DMA_IT_HT) != 0U)
      {
        // Clear the half transfer complete flag 
        regs_dma->IFCR = DMA_FLAG_HTIF0_4 << (hdma->StreamIndex & 0x1FU);

        // Disable the half transfer interrupt if the DMA mode is not CIRCULAR 
        if((((DMA_Stream_TypeDef   *)hdma->Instance)->CR & DMA_SxCR_CIRC) == 0U)
        {
          // Disable the half transfer interrupt 
          ((DMA_Stream_TypeDef   *)hdma->Instance)->CR  &= ~(DMA_IT_HT);
        }

      }
      interrupt = DMA_HALF_TRANSFER;
    }
    // Transfer Complete Interrupt management **********************************
    if ((tmpisr_dma & (DMA_FLAG_TCIF0_4 << (hdma->StreamIndex & 0x1FU))) != 0U)
    {
      if(__HAL_DMA_GET_IT_SOURCE(hdma, DMA_IT_TC) != 0U)
      {
        // Clear the transfer complete flag 
        regs_dma->IFCR = DMA_FLAG_TCIF0_4 << (hdma->StreamIndex & 0x1FU);

        if(HAL_DMA_STATE_ABORT == hdma->State)
        {
          // Disable all the transfer interrupts 
          ((DMA_Stream_TypeDef   *)hdma->Instance)->CR  &= ~(DMA_IT_TC | DMA_IT_TE | DMA_IT_DME);
          ((DMA_Stream_TypeDef   *)hdma->Instance)->FCR &= ~(DMA_IT_FE);

          if((hdma->XferHalfCpltCallback != NULL) || (hdma->XferM1HalfCpltCallback != NULL))
          {
            ((DMA_Stream_TypeDef   *)hdma->Instance)->CR  &= ~(DMA_IT_HT);
          }

          // Clear all interrupt flags at correct offset within the register 
          regs_dma->IFCR = 0x3FUL << (hdma->StreamIndex & 0x1FU);

          // Change the DMA state 
          hdma->State = HAL_DMA_STATE_READY;

          // Process Unlocked 
          __HAL_UNLOCK(hdma);

          interrupt = DMA_TRANSFER_COMPLETE;
        }

        if((((DMA_Stream_TypeDef   *)hdma->Instance)->CR & DMA_SxCR_CIRC) == 0U)
        {
          // Disable the transfer complete interrupt 
          ((DMA_Stream_TypeDef   *)hdma->Instance)->CR  &= ~(DMA_IT_TC);

          // Change the DMA state 
          hdma->State = HAL_DMA_STATE_READY;

          // Process Unlocked 
          __HAL_UNLOCK(hdma);
        }
        interrupt =  2;
      }
    }
  }

  return interrupt;
}

int STM32F4_SPIComms::getActiveDMAmemory(DMA_HandleTypeDef *hdma)
{
    DMA_Stream_TypeDef *dmaStream = (DMA_Stream_TypeDef *)hdma->Instance;

    return (dmaStream->CR & DMA_SxCR_CT) ? 1 : 0;
}

void STM32F4_SPIComms::handleNssInterrupt()
{
	// SPI packet has been fully received
	// Flag the copy the RX buffer if new WRITE data has been received
	// DMA copy is performed during the servo thread update
	if (newWriteData)
	{
		copyRXbuffer = true;
		newWriteData = false;
	}
}

void STM32F4_SPIComms::handleTxInterrupt()
{
	handleDMAInterrupt(&hdma_spi_tx);
	HAL_NVIC_EnableIRQ(irqDMAtx);
}

void STM32F4_SPIComms::handleRxInterrupt()
{
    // Handle the interrupt and determine the type of interrupt
    interruptType = handleDMAInterrupt(&hdma_spi_rx);

    RxDMAmemoryIdx = getActiveDMAmemory(&hdma_spi_rx);

    if (interruptType == DMA_HALF_TRANSFER) // Use the HTC interrupt to check the packet being received
    {
        switch (ptrRxDMABuffer->buffer[RxDMAmemoryIdx].header)
        {
            case Config::pruRead:
                // No action needed for PRU_READ.
            	dataCallback(true);
                break;

            case Config::pruWrite:
            	// Valid PRU_WRITE header, flag RX data transfer.
            	dataCallback(true);
            	newWriteData = true;
                RXbufferIdx = RxDMAmemoryIdx;
                break;

            default:
            	dataCallback(false);
                break;
        }
    }
    else if (interruptType == DMA_TRANSFER_COMPLETE) // Transfer complete interrupt
    {
        // Placeholder for transfer complete handling if needed in the future.
    }
    else // Other interrupt sources
    {
        printf("DMA SPI Rx error\n");
    }

    HAL_NVIC_EnableIRQ(irqDMArx);
}

void STM32F4_SPIComms::tasks() {

	if (copyRXbuffer == true)
    {
	    uint8_t* srcBuffer = (uint8_t*)ptrRxDMABuffer->buffer[RXbufferIdx].rxBuffer;
	    uint8_t* destBuffer = (uint8_t*)ptrRxData->rxBuffer;

	    __disable_irq();

	    dmaStatus = HAL_DMA_Start( 
	    							&hdma_memtomem,
									(uint32_t)srcBuffer,
									(uint32_t)destBuffer,
									Config::dataBuffSize
	    							);

	    if (dmaStatus == HAL_OK) {
	        dmaStatus = HAL_DMA_PollForTransfer(&hdma_memtomem, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
	    }

	    __enable_irq();
	    HAL_DMA_Abort(&hdma_memtomem);
		copyRXbuffer = false;
    }
}

static void MultiBufferDMAErrorCallback(DMA_HandleTypeDef *hdma)
{
    Error_Handler();  // Stop execution or handle gracefully
}