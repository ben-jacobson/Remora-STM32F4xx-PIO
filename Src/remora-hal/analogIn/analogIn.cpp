#include "analogIn.h"
#include <cstdio>

AnalogIn::AnalogIn(const std::string& portAndPin) 
    : portAndPin(portAndPin)
{
    uint32_t function = (uint32_t)NC;

    if (portAndPin[0] == 'P') {
        portIndex = portAndPin[1] - 'A';
        pinNumber = std::stoi(portAndPin.substr(3));
        pin = 1 << pinNumber;
    } else {
        printf("Invalid port and pin definition\n");
        return;
    }

    pinName = static_cast<PinName>((portIndex << 4) | pinNumber);

    // Get ADC instance and channel function
    handle.Instance = (ADC_TypeDef *)pinmap_peripheral(pinName, PinMap_ADC);
    function = pinmap_function(pinName, PinMap_ADC);

    analogInPin = new Pin(portAndPin, STM_PIN_FUNCTION(function));
    channel = STM_PIN_CHANNEL(function);
    differential = STM_PIN_INVERTED(function);

    // Configure ADC settings
    handle.Init.Resolution = ADC_RESOLUTION_12B;
    handle.Init.ScanConvMode = DISABLE;
    handle.Init.ContinuousConvMode = DISABLE;
    handle.Init.DiscontinuousConvMode = DISABLE;
    handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    handle.Init.NbrOfConversion = 1;
    handle.Init.DMAContinuousRequests = DISABLE;
    handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;    
    //enableADCClock(handle.Instance);

    if (HAL_ADC_Init(&handle) != HAL_OK) {
        printf("HAL_ADC_Init failed\n");
    }

    // Configure multimode for ADC1
    if (handle.Instance == ADC1) {
        ADC_MultiModeTypeDef multimode = {0};
        multimode.Mode = ADC_MODE_INDEPENDENT;
        if (HAL_ADCEx_MultiModeConfigChannel(&handle, &multimode) != HAL_OK) {
            printf("HAL_ADCEx_MultiModeConfigChannel failed\n");
        }
    }
}

uint16_t AnalogIn::read()
{
    // Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time
    ADC_ChannelConfTypeDef sConfig = {0}; 

    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    if (HAL_ADC_ConfigChannel(&handle, &sConfig) != HAL_OK)
    {
        printf("HAL_ADC_ConfigChannel failed\n");
    }    

    if (HAL_ADC_Start(&handle) != HAL_OK) {
        printf("HAL_ADC_Start failed\n");
    }

    uint16_t adcValue = 0;
    if (HAL_ADC_PollForConversion(&handle, 10) == HAL_OK) {
        adcValue = static_cast<uint16_t>(HAL_ADC_GetValue(&handle));
    }

    LL_ADC_SetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(handle.Instance), LL_ADC_PATH_INTERNAL_NONE);

    if (HAL_ADC_Stop(&handle) != HAL_OK) {
        printf("HAL_ADC_Stop failed\n");
    }

    return adcValue;
}

// --- Helper functions ---

uint32_t AnalogIn::getADCChannelConstant(int channel)
{
    static const uint32_t channelTable[] = {
        ADC_CHANNEL_0, ADC_CHANNEL_1, ADC_CHANNEL_2, ADC_CHANNEL_3,
        ADC_CHANNEL_4, ADC_CHANNEL_5, ADC_CHANNEL_6, ADC_CHANNEL_7,
        ADC_CHANNEL_8, ADC_CHANNEL_9, ADC_CHANNEL_10, ADC_CHANNEL_11,
        ADC_CHANNEL_12, ADC_CHANNEL_13, ADC_CHANNEL_14, ADC_CHANNEL_15,
        ADC_CHANNEL_16, ADC_CHANNEL_17, ADC_CHANNEL_18
    };

    if (channel < 0 || channel >= static_cast<int>(sizeof(channelTable) / sizeof(channelTable[0]))) {
        printf("Invalid ADC channel %d\n", channel);
        return 0;
    }

    return channelTable[channel];
}

void AnalogIn::enableADCClock(ADC_TypeDef* adc)
{
    if (adc == ADC1 || adc == ADC2) {
        __HAL_RCC_ADC2_CLK_ENABLE();
    } else if (adc == ADC3) {
        __HAL_RCC_ADC3_CLK_ENABLE();
    } else {
        printf("Unknown ADC instance\n");
    }
}
