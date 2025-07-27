/*
Remora firmware for LinuxCNC
Copyright (C) 2025  Scott Alford (aka scotta)

STM32F4 Port by Ben Jacobson. 
Credits to Cakeslob and Expatria Technologies for their Ethernet communications work ported into this project.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "main.h"

#include <stdio.h>
#include <cstring>
#include <sys/errno.h>

#include "remora-core/remora.h"

#if defined(CONTROL_METHOD) && (CONTROL_METHOD == SPI_CTRL)
  #include "remora-hal/STM32F4_SPIComms.h"
#else if defined(CONTROL_METHOD) && (CONTROL_METHOD == ETH_CTRL)
  #include "remora-hal/STM32F4_EthComms.h"
#endif

#include "remora-hal/STM32F4_timer.h"

SPI_HandleTypeDef spi_handle;
UART_HandleTypeDef uart_handle;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_UART_Init(void);

// re-target printf to defined UART by redeclaring weak function in syscalls.c
extern "C" {
    int _write(int file, char *ptr, int len) {
        HAL_UART_Transmit(&uart_handle, (uint8_t*)ptr, len, HAL_MAX_DELAY);
        return len;
    }
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_UART_Init();

    HAL_Delay(1000); 
    printf("Initialising Remora...\n");

    std::unique_ptr<CommsInterface> comms;
    std::shared_ptr<CommsHandler> commsHandler;

    #if defined(CONTROL_METHOD) && (CONTROL_METHOD == SPI_CTRL)
        comms = std::make_unique<STM32F4_SPIComms>(&rxData, &txData, SPI_MOSI, SPI_MISO, SPI_CLK, SPI_CS);
        commsHandler = std::make_shared<CommsHandler>();
    #elif defined(CONTROL_METHOD) && (CONTROL_METHOD == ETH_CTRL)
        #ifndef WIZ_RST
            #error "Please configure your WIZ_RST pin in platformio.ini"
        #endif
        comms = std::make_unique<STM32F4_EthComms>(&rxData, &txData, SPI_MOSI, SPI_MISO, SPI_CLK, SPI_CS, WIZ_RST);
        commsHandler = std::make_shared<CommsHandler>();
    #else 
        #error "Error in configuration, please set PlatformIO.ini build flag or set pruControlMethod in Configuration.h, to either SPI_CTRL or ETH_CTRL";
    #endif

    commsHandler->setInterface(std::move(comms));

    auto baseTimer = std::make_unique<STM32F4_timer>(TIM3, TIM3_IRQn, Config::pruBaseFreq, nullptr, Config::baseThreadIrqPriority);
    auto servoTimer = std::make_unique<STM32F4_timer>(TIM2, TIM2_IRQn, Config::pruServoFreq, nullptr, Config::servoThreadIrqPriority);
    auto serialTimer = std::make_unique<STM32F4_timer>(TIM4, TIM4_IRQn, Config::pruSerialFreq, nullptr, Config::serialThreadIrqPriority);

    Remora* remora = new Remora(
            commsHandler,
            std::move(baseTimer),
            std::move(servoTimer),
            std::move(serialTimer)
    );

    remora->run();
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_UART_Init(void)
{
    #ifdef UART_PORT
        uart_handle.Instance = UART_PORT;
    #else
        uart_handle.Instance = USART2;
    #endif
    uart_handle.Init.BaudRate = Config::pcBaud;
    uart_handle.Init.WordLength = UART_WORDLENGTH_8B;
    uart_handle.Init.StopBits = UART_STOPBITS_1;
    uart_handle.Init.Parity = UART_PARITY_NONE;
    uart_handle.Init.Mode = UART_MODE_TX_RX;
    uart_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    uart_handle.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&uart_handle) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
