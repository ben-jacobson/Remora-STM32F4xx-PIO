#ifndef HARDWAREPWM_H
#define HARDWAREPWM_H

#include <string>
#include <iostream>

#include "stm32f4xx_hal.h"
#include "../hal_utils.h"

#define DEFAULT_PWMPERIOD 200
//#define ONE_MHZ 1000000 

// struct PWM_Enabled_Pin {
//     std::string pin_name;         
//     GPIO_TypeDef* gpio_port;
//     uint16_t gpio_pin;
//     TIM_TypeDef* timer;
//     uint32_t channel; 
// };

//extern PWM_Enabled_Pin pwm_enabled_pins[];
//PWM_Enabled_Pin* find_compatible_pwm_pin(std::string); 

class HardwarePWM
{
	private:
        TIM_ClockConfigTypeDef sClockSourceConfig = {0};
        TIM_MasterConfigTypeDef sMasterConfig = {0};
        TIM_OC_InitTypeDef sConfigOC = {0};
        TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

        std::string pwm_pin_str;			     
        PinName pwm_pin_name;
        Pin* pwm_pin;
        PWMName pwm_name;

        TIM_HandleTypeDef pwm_tim_handler;
        uint32_t pwm_tim_channel_used; 
        //PWM_Enabled_Pin *configured_pin;

        uint32_t timer_clk_hz;

        void setTimerAndChannelInstance(uint32_t);
        void initialise_timers(void);
        void initialise_pwm_channels(void);
        void initialise_pwm_pin_clocks(void);

	public:
        HardwarePWM(int, int, std::string);
		~HardwarePWM(void);			        
        void change_period(int);
        void change_pulsewidth(int);
};

#endif