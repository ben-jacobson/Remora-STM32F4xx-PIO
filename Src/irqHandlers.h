#ifndef IRQ_HANDLERS_H
#define IRQ_HANDLERS_H

#include "remora-core/interrupt/interrupt.h"
#include "stm32f4xx_hal.h"
#include "remora-hal/STM32F4_EthComms.h"

extern "C" {

    void EXTI4_IRQHandler() {
        if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_4) != RESET) {
            __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);
            Interrupt::InvokeHandler(EXTI4_IRQn);
        }
    }

    // void DMA1_Stream0_IRQHandler() {     // Need to set include guards to renable SPIComms DMA
    //     Interrupt::InvokeHandler(DMA1_Stream0_IRQn);
    // }

    // void DMA1_Stream1_IRQHandler() {
    //     Interrupt::InvokeHandler(DMA1_Stream1_IRQn);
    // }

    void DMA2_Stream0_IRQHandler(void) // SPI 1 RX
    {
        HAL_DMA_IRQHandler(&STM32F4_EthComms::hdma_spi_rx);
    }

    void DMA1_Stream3_IRQHandler(void)
    {
        HAL_DMA_IRQHandler(&STM32F4_EthComms::hdma_spi_rx);
    }

    void DMA1_Stream0_IRQHandler(void)
    {
        HAL_DMA_IRQHandler(&STM32F4_EthComms::hdma_spi_rx);
    }

    void DMA2_Stream3_IRQHandler(void) // SPI 1 TX
    {  
        HAL_DMA_IRQHandler(&STM32F4_EthComms::hdma_spi_tx);    
    }

    void DMA1_Stream4_IRQHandler(void)
    { 
        HAL_DMA_IRQHandler(&STM32F4_EthComms::hdma_spi_tx);    
    }

    void DMA1_Stream5_IRQHandler(void)
    {  
        HAL_DMA_IRQHandler(&STM32F4_EthComms::hdma_spi_tx);    
    }

    void SPI1_IRQHandler(void)
    {
        HAL_SPI_IRQHandler(&STM32F4_EthComms::spiHandle);
    }

    void SPI2_IRQHandler(void)
    {
        HAL_SPI_IRQHandler(&STM32F4_EthComms::spiHandle);
    }

    void SPI3_IRQHandler(void)
    {
        HAL_SPI_IRQHandler(&STM32F4_EthComms::spiHandle);        
    }    

    void TIM2_IRQHandler() {
        if (TIM2->SR & TIM_SR_UIF) {
            TIM2->SR &= ~TIM_SR_UIF;
            Interrupt::InvokeHandler(TIM2_IRQn);
        }
    }

    void TIM3_IRQHandler() {
        if (TIM3->SR & TIM_SR_UIF) {
            TIM3->SR &= ~TIM_SR_UIF;
            Interrupt::InvokeHandler(TIM3_IRQn);
        }
    }

    void TIM4_IRQHandler() {
        if (TIM4->SR & TIM_SR_UIF) {
            TIM4->SR &= ~TIM_SR_UIF;
            Interrupt::InvokeHandler(TIM4_IRQn);
        }
    }

} // extern "C"

#endif // IRQ_HANDLERS_H