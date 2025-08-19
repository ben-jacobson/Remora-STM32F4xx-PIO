#include "hardware_pwm.h"

uint32_t get_timer_clk_freq(TIM_TypeDef* TIMx)
{
    uint32_t pclk, multiplier;

    if (TIMx == TIM1 || TIMx == TIM8 || TIMx == TIM9 || TIMx == TIM10 || TIMx == TIM11) 
    {       
        // On APB2
        pclk = HAL_RCC_GetPCLK2Freq();
        uint32_t ppre2 = ((RCC->CFGR & RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_Pos);
        multiplier = (ppre2 < 4) ? 1 : 2;
    }
    else    
    {
        // On APB1
        pclk = HAL_RCC_GetPCLK1Freq();
        uint32_t ppre1 = ((RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos);
        multiplier = (ppre1 < 4) ? 1 : 2;       
    }
    return pclk * multiplier;
}

HardwarePWM::HardwarePWM(int _initial_period_us, int _initial_pulsewidth_us, std::string _pwm_pin_str):
	pwm_pin_str(_pwm_pin_str)
{
    pwm_pin_name = portAndPinToPinName(pwm_pin_str.c_str());   
    uint32_t pwm_function = pinmap_function(pwm_pin_name, PinMap_PWM);

    if (pwm_function != (uint32_t)NC)
    {
        setTimerAndChannelInstance(pwm_function); // sets pwm_tim_handler.Instance and the channel from the pinmap

        if (pwm_tim_handler.Instance == TIM2 ||     // TODO - refactor once the configured timers are visible in the configuration.h file
            pwm_tim_handler.Instance == TIM3) 
        {
            printf("Warning: Timer instance clashes with one of the timers used for Remora BASE or SERVO threads, this crash the PRU");
        }

        // Initialise the pin clocks, timer, channel and pin
        initialise_pwm_pin_clocks();
        initialise_timers();
        initialise_pwm_channels();  
        pwm_pin = createPinFromPinMap(pwm_pin_str, pwm_pin_name, PinMap_PWM, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW);  // reenable after test

        // check that initialisation worked.
        if (HAL_TIM_PWM_Start(&pwm_tim_handler, pwm_tim_channel_used) != HAL_OK) 
        {
            Error_Handler();
        }  

        // set up default values. 
        change_period(_initial_period_us);
        change_pulsewidth(_initial_pulsewidth_us);

        //printf("Timer clk frequency: %lu Hz\n", this->timer_clk_hz);  // debug if needed
        //printf("Prescaler: %lu\n", this->pwm_tim_handler.Init.Prescaler);
    }
    else
    {
        printf("Error, could not set up pin %s as PWM. Please check the available list of PWM pins on your hardware target and try again\n", pwm_pin_str.c_str());
    }    
}

void HardwarePWM::setTimerAndChannelInstance(uint32_t pwm_pinmap_function)
{
    // Cast the peripheral pin style timer to STM style timer
    pwm_tim_handler.Instance = ((TIM_TypeDef *)getPWMName(pwm_pin_name));

    // reverse the stored encoding of the function to derive the channel used
    uint32_t pin_function_timer_channel = ((pwm_pinmap_function) >> STM_PIN_CHAN_SHIFT) & STM_PIN_CHAN_MASK;

    switch (pin_function_timer_channel) 
    {
        case 1: 
            pwm_tim_channel_used = TIM_CHANNEL_1;   
            break;        
        case 2: 
            pwm_tim_channel_used = TIM_CHANNEL_2;   
            break;
        case 3: 
            pwm_tim_channel_used = TIM_CHANNEL_3;   
            break;
        case 4: 
            pwm_tim_channel_used = TIM_CHANNEL_4;   
            break;
    }
}

void HardwarePWM::initialise_timers(void) 
{
    // note that pwm_tim_handler.Instance and pwm_tim_channel_used are set prior to calling this in constructor.  
    timer_clk_hz = get_timer_clk_freq(pwm_tim_handler.Instance); 
    pwm_tim_handler.Init.Prescaler = 0;
    pwm_tim_handler.Init.CounterMode = TIM_COUNTERMODE_UP;
    pwm_tim_handler.Init.Period = 0xffff; 
    pwm_tim_handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    pwm_tim_handler.Init.RepetitionCounter = 0;
    pwm_tim_handler.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    
    if (HAL_TIM_Base_Init(&pwm_tim_handler) != HAL_OK)
    {
        Error_Handler(); 
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    
    if (HAL_TIM_ConfigClockSource(&pwm_tim_handler, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    
    if (HAL_TIM_PWM_Init(&pwm_tim_handler) != HAL_OK)
    {
        Error_Handler();
    }
    
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    
    if (HAL_TIMEx_MasterConfigSynchronization(&pwm_tim_handler, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

void HardwarePWM::initialise_pwm_channels(void) 
{
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = pwm_tim_handler.Init.Period * 0.5; // gives this an inital value for sanity checking, will be overwritten on startup. 
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    
    if (HAL_TIM_PWM_ConfigChannel(&pwm_tim_handler, &sConfigOC, pwm_tim_channel_used) != HAL_OK)
    {
        Error_Handler();
    }

    sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
    sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime = 0;  // We may need to adjust this for safety when using PWM to drive higher current loads. Obvious use case is spindle PWM, but if ever used for anything else may need to be revisited
    sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
    sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
    
    if (HAL_TIMEx_ConfigBreakDeadTime(&pwm_tim_handler, &sBreakDeadTimeConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

void HardwarePWM::initialise_pwm_pin_clocks(void) 
{
    if (pwm_tim_handler.Instance == TIM1) 
        __HAL_RCC_TIM1_CLK_ENABLE();
    else if (pwm_tim_handler.Instance == TIM2) 
        __HAL_RCC_TIM2_CLK_ENABLE();
    else if (pwm_tim_handler.Instance == TIM3) 
        __HAL_RCC_TIM3_CLK_ENABLE();
    else if (pwm_tim_handler.Instance == TIM4) 
        __HAL_RCC_TIM4_CLK_ENABLE();
    else if (pwm_tim_handler.Instance == TIM5) 
        __HAL_RCC_TIM5_CLK_ENABLE();
    else if (pwm_tim_handler.Instance == TIM6) 
        __HAL_RCC_TIM6_CLK_ENABLE();
    else if (pwm_tim_handler.Instance == TIM7) 
        __HAL_RCC_TIM7_CLK_ENABLE();
    else if (pwm_tim_handler.Instance == TIM8) 
        __HAL_RCC_TIM8_CLK_ENABLE();
    else if (pwm_tim_handler.Instance == TIM9) 
        __HAL_RCC_TIM9_CLK_ENABLE();
    else if (pwm_tim_handler.Instance == TIM10) 
        __HAL_RCC_TIM10_CLK_ENABLE();
    else if (pwm_tim_handler.Instance == TIM11) 
        __HAL_RCC_TIM11_CLK_ENABLE();
    else if (pwm_tim_handler.Instance == TIM12) 
        __HAL_RCC_TIM12_CLK_ENABLE();
    else if (pwm_tim_handler.Instance == TIM13) 
        __HAL_RCC_TIM13_CLK_ENABLE();
    else if (pwm_tim_handler.Instance == TIM14) 
        __HAL_RCC_TIM14_CLK_ENABLE();
    else 
        printf("incorrect timer selected, please refer to documentation.\n");        
}

HardwarePWM::~HardwarePWM(void) 
{
    if (pwm_tim_handler.Instance == TIM1)  
        __HAL_RCC_TIM1_CLK_DISABLE();
    if (pwm_tim_handler.Instance == TIM2)  
        __HAL_RCC_TIM2_CLK_DISABLE();
    if (pwm_tim_handler.Instance == TIM3)  
        __HAL_RCC_TIM3_CLK_DISABLE();
    if (pwm_tim_handler.Instance == TIM4)  
        __HAL_RCC_TIM4_CLK_DISABLE();
    if (pwm_tim_handler.Instance == TIM5)  
        __HAL_RCC_TIM5_CLK_DISABLE();
    if (pwm_tim_handler.Instance == TIM6)  
        __HAL_RCC_TIM6_CLK_DISABLE();
    if (pwm_tim_handler.Instance == TIM7)  
        __HAL_RCC_TIM7_CLK_DISABLE();
    if (pwm_tim_handler.Instance == TIM8)  
        __HAL_RCC_TIM8_CLK_DISABLE();
    if (pwm_tim_handler.Instance == TIM9)  
        __HAL_RCC_TIM9_CLK_DISABLE();
    if (pwm_tim_handler.Instance == TIM10) 
        __HAL_RCC_TIM10_CLK_DISABLE();
    if (pwm_tim_handler.Instance == TIM11) 
        __HAL_RCC_TIM11_CLK_DISABLE();
    if (pwm_tim_handler.Instance == TIM12) 
        __HAL_RCC_TIM12_CLK_DISABLE();
    if (pwm_tim_handler.Instance == TIM13) 
        __HAL_RCC_TIM13_CLK_DISABLE();
    if (pwm_tim_handler.Instance == TIM14) 
        __HAL_RCC_TIM14_CLK_DISABLE();       
}

void HardwarePWM::change_period(int new_period_us)
{
    uint32_t timer_freq_after_prescaler = timer_clk_hz / (pwm_tim_handler.Init.Prescaler + 1);
    uint32_t period_ticks = (timer_freq_after_prescaler * new_period_us) / 1000000;

    if (period_ticks < 1) 
    {
        period_ticks = 1;
    }

    pwm_tim_handler.Init.Period = period_ticks - 1;

    // attempt to restart PWM with minimal interruption, inclduing recalculation of pulse width
    __HAL_TIM_DISABLE(&pwm_tim_handler);
    __HAL_TIM_SET_AUTORELOAD(&pwm_tim_handler, period_ticks - 1);

    // Ensure pulse width is not out of bounds of new period value, otherwise the counter will never reach it. 
    if (__HAL_TIM_GET_COMPARE(&pwm_tim_handler, pwm_tim_channel_used) > (period_ticks - 1)) {
        __HAL_TIM_SET_COMPARE(&pwm_tim_handler, pwm_tim_channel_used, period_ticks - 1);
    }

    pwm_tim_handler.Instance->EGR |= TIM_EGR_UG; // trigger reload.

    // re-enable
    __HAL_TIM_ENABLE(&pwm_tim_handler); 
}

void HardwarePWM::change_pulsewidth(int new_pulsewidth_us)
{
    uint32_t timer_freq_after_prescaler = timer_clk_hz / (pwm_tim_handler.Init.Prescaler + 1);
    uint32_t pulse_ticks = (timer_freq_after_prescaler * new_pulsewidth_us) / 1000000;    

    // clamp it
    if (pulse_ticks < 1) 
    {
        pulse_ticks = 0;
    }
    if (pulse_ticks > pwm_tim_handler.Init.Period)    
    {
        pulse_ticks = pwm_tim_handler.Init.Period;
    }

    __HAL_TIM_SET_COMPARE(&pwm_tim_handler, pwm_tim_channel_used, pulse_ticks);
}