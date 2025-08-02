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

    #ifdef SPI_CTRL // these need to be remapped - the F4 isn't as flexible as the H7 and has fixed streams available for different SPI buses. We have this set up in the ETHComms, but they don't link back to the DMA IRQs
    void DMA1_Stream0_IRQHandler() {  
        Interrupt::InvokeHandler(DMA1_Stream0_IRQn);
    }

    void DMA1_Stream1_IRQHandler() {
        Interrupt::InvokeHandler(DMA1_Stream1_IRQn);
    }
    #endif


    #ifdef ETH_CTRL

    // macro for automatically creating the DMA Stream IRQ handlers, e.g DMA2_Stream0_IRQHandler()
    #define DMA_STREAM_IRQ_HANDLER(DMA_num, DMA_stream, IRQ_n)      \
    void DMA##DMA_num##_Stream##DMA_stream##_IRQHandler(void) {     \
        Interrupt::InvokeHandler(IRQ_n);                            \
    }

    DMA_STREAM_IRQ_HANDLER(2, 0, DMA2_Stream0_IRQn) // SPI 1 RX
    DMA_STREAM_IRQ_HANDLER(1, 3, DMA1_Stream3_IRQn) 
    DMA_STREAM_IRQ_HANDLER(1, 0, DMA1_Stream0_IRQn) 
    DMA_STREAM_IRQ_HANDLER(2, 3, DMA2_Stream3_IRQn) // SPI 1 TX
    DMA_STREAM_IRQ_HANDLER(1, 4, DMA1_Stream4_IRQn) 
    DMA_STREAM_IRQ_HANDLER(1, 5, DMA1_Stream5_IRQn) 

    // void DMA2_Stream0_IRQHandler(void) // SPI 1 RX
    // {
    //     Interrupt::InvokeHandler(DMA2_Stream0_IRQn);
    // }

    // void DMA1_Stream3_IRQHandler(void)
    // {
    //     Interrupt::InvokeHandler(DMA1_Stream3_IRQn);
    // }

    // void DMA1_Stream0_IRQHandler(void)
    // {
    //     Interrupt::InvokeHandler(DMA1_Stream0_IRQn);
    // }

    // void DMA2_Stream3_IRQHandler(void) // SPI 1 TX
    // {  
    //     Interrupt::InvokeHandler(DMA2_Stream3_IRQn); 
    // }

    // void DMA1_Stream4_IRQHandler(void)
    // { 
    //     Interrupt::InvokeHandler(DMA1_Stream4_IRQn);
    // }

    // void DMA1_Stream5_IRQHandler(void)
    // {  
    //     Interrupt::InvokeHandler(DMA1_Stream5_IRQn); 
    // }

    // macro for automatically creating the SPI IRQ handlers, e.g SPI1_IRQHandler()
    #define SPI_IRQ_HANDLER(SPI_IRQ_FUNCTION_NAME, SPI_IRQ_NUM) \
    void SPI_IRQ_FUNCTION_NAME(void) {                          \
        Interrupt::InvokeHandler(SPI_IRQ_NUM);                  \
    }

    SPI_IRQ_HANDLER(SPI1_IRQHandler, SPI1_IRQn)
    SPI_IRQ_HANDLER(SPI2_IRQHandler, SPI2_IRQn)
    SPI_IRQ_HANDLER(SPI3_IRQHandler, SPI3_IRQn)

    // void SPI1_IRQHandler(void)
    // {
    //     Interrupt::InvokeHandler(SPI1_IRQn); 
    // }

    // void SPI2_IRQHandler(void)
    // {
    //     Interrupt::InvokeHandler(SPI2_IRQn); 
    // }

    // void SPI3_IRQHandler(void)
    // {
    //     Interrupt::InvokeHandler(SPI3_IRQn); 
    // }    

    #endif

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