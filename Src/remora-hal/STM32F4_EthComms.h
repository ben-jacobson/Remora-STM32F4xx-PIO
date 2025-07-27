#ifndef STM32F4_ETHComms_H
#define STM32F4_ETHComms_H

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_dma.h"

#include <stdio.h>
#include <string.h>
#include <memory>

#include "../remora-core/remora.h"
#include "../remora-core/comms/commsInterface.h"
#include "../remora-core/modules/moduleInterrupt.h"
#include "hal_utils.h"

// typedef struct
// {
//   __IO uint32_t ISR;   //!< DMA interrupt status register 
//   __IO uint32_t Reserved0;
//   __IO uint32_t IFCR;  //!< DMA interrupt flag clear register 
// } DMA_Base_Registers;

// typedef enum {
//     DMA_HALF_TRANSFER = 1,   // Half-transfer completed
//     DMA_TRANSFER_COMPLETE = 2, // Full transfer completed
//     DMA_OTHER = 3        // Other or error status
// } DMA_TransferStatus_t;

class STM32F4_EthComms : public CommsInterface {
    private:
        volatile DMA_RxBuffer_t* 	ptrRxDMABuffer;

        // SPI_TypeDef*        		spiType;
        SPI_HandleTypeDef   		spiHandle;
        DMA_HandleTypeDef   		hdma_spi_tx;
        DMA_HandleTypeDef   		hdma_spi_rx;
        DMA_HandleTypeDef   		hdma_memtomem;
        HAL_StatusTypeDef   		dmaStatus;

        std::string                 mosiPortAndPin; 
        std::string                 misoPortAndPin; 
        std::string                 clkPortAndPin; 
        std::string                 csPortAndPin; 
        std::string                 rstPortAndPin;

        PinName                     mosiPinName;
        PinName                     misoPinName;
        PinName                     clkPinName;
        PinName                     csPinName;
        PinName                     rstPinName;


        Pin*                        mosiPin;
        Pin*                        misoPin;
        Pin*                        clkPin;
        Pin*                        csPin;
        Pin*                        rstPin;

        uint8_t						RxDMAmemoryIdx;
        uint8_t						RXbufferIdx;
        bool						newDataReceived;

        // ModuleInterrupt<STM32F4_EthComms>*	NssInterrupt;
        // ModuleInterrupt<STM32F4_EthComms>*	dmaTxInterrupt;
        // ModuleInterrupt<STM32F4_EthComms>*	dmaRxInterrupt;

        // IRQn_Type					irqNss;
        // IRQn_Type					irqDMArx;
        // IRQn_Type					irqDMAtx;

        // uint8_t						interruptType;
        // bool						newWriteData;

        void initDMA(DMA_Stream_TypeDef* DMA_RX_Stream, DMA_Stream_TypeDef* DMA_TX_Stream, uint32_t DMA_channel);

        // HAL_StatusTypeDef startMultiBufferDMASPI(uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint16_t);
        // int getActiveDMAmemory(DMA_HandleTypeDef*);

        // int handleDMAInterrupt(DMA_HandleTypeDef *);
        // void handleRxInterrupt(void);
        // void handleTxInterrupt(void);
        // void handleNssInterrupt(void);
        
    public:   
        STM32F4_EthComms(volatile rxData_t*, volatile txData_t*, std::string, std::string, std::string, std::string, std::string);
        virtual ~STM32F4_EthComms();
        void dataReceived(void);

        void init(void);
        void start(void);
        void tasks(void);
};

#endif

