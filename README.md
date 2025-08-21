# Remora-STM32F4xx-PIO
Port or Remora for STM32F4xx family, ported to use the latest Remora-Core abstraction layer.

# Todos / Status
- Network control code - in progress:
    - Current status:
        - Wiznet and LWIP integrated
        - Connects to the network
        - Using the Remora-Eth-3.0 LinuxCNC component, able to switch the PRU in and out of eStop modes.
        - TFTP is listening and will process config uploads
    - Todos:
        - Improve testing around lost packet checks
- JSON config loading - in progress:
    - Current status:
        - Working - No known issues 
- SPI Comms - in progress:
    - Current status:
        - Sign of life, but not fully tested - currently working with an RPI2 which isn't supported. Can toggle estop in and out which is promising.
    - Todos:
        - Buy or borrow an RPI4
        - Buy an SD card module for testing native FATFS reading. 
        - Full test required for new code introduced prior to mounting the file system 
        - Refactoring of Mbed code used for handling DMA interrupts. See if we can replace with a set of idiomatic STM HAL DMA handlers
- Uart3 support - in progress: 
    - Current status:
        - Basic set up for console logging on a select peripheral set in PlatformIO.ini
    - Todos:
        - Refactor so that this is handled by a class. Perhaps use the comms interface? 
- Modules - in progress:
    - Blinky: Working - no known issues
    - Stepgen, digital IO, Analog Ins - to be tested
    - Hardware PWM  to be ported in from deprecated F4 code base
    - Encoder modules to be ported in.
    - Linker scripts to be built for a more extensive range of F4xx builds
    - Software PWM to be ported in
- Adhoc todos: 
    - Do we need to wrap up the SDIO implementation like we will for UART? unsure how configurable this needs to be across the family. 
    - Relook at the note inside the configuration.h file, it references outdated syntax for how the file is to be built. 
    - ADC - move MSP init code into class
    - Clear up conflict in DMA IRQs between SPI and Ethernet comms modules, just some simple include guarding will suffice.
    - Test out differnet clock config code, what's different between the boards?
    - Look at IRQHandle and move the DMAStreams over to unused channel/stream combos for SPIComms on F4
    - Build the bootloader linker scripts
    - Alter EthComms component to use same payloads as SPI. 

# Wiznet W5500 connection
- PA_5: SCK
- PA_6: MISO
- PA_7: MOSI
- WIZ_RST: configured in Platformio.ini. Default = PB_5
- SPI_CS: configured in Platformio.ini. Default = PB_6

# UART connections
- UART_TX: PD_9
- UART_RX: PD_8 - Firmware doesn't make use of this, but left for completeness and future expansion

# Allocation of Step Generators, IO and PWM
Please refer to the Remora documentation to configure GPIO to perform various functions like stepgen, digital IO and PWM: https://remora-docs.readthedocs.io/en/latest/configuration/configuration.html
Example config.txt files can be found in the LinuxCNC_Configs folder. 

# Hardware PWM
Hardware PWM is available on a wide variety of pins depending on your hardware target. When setting up your config.txt file, you must choose a PWM enabled pin from the list provided. Specific STM32 Timers and Channels will been allocated by the driver behind the scenes. Some important details about this: 
- PWM pins can be set to variable or fixed period. Configuration documentation can be found here https://remora-docs.readthedocs.io/en/latest/configuration/Setup-Config-File.html#pwm
- You may set up more than one PWM pin on the same timer (TIMx) as each pin is assigned a different channel, however sharing TIMx's has some interesting nuances you must be aware of:
    - The period setting is shared across that TIMx so configuring it or changing it will effect all four channels. Any PWM pins on that TIMx will be effected with whatever period you set in config.txt or if you vary the period at run time. 
    - Due to above, setting some PWM pins on the same TIMx with variable period and others with fixed period defeats its own purpose. Changes to any variable period will change all channels on that TIMx. Do note that we've been careful to force recalculation of the duty cycle on period change so this may not cause a problem. It will depend on your application. 
    - Note that some of the channels below are marked with an N at the end. These are inverted copies of their non-N counterparts. They can be used individually or paired up which is great for driving differential signals. As an example, if you configure a PWM channel on PA_9 (TIM1_CH2) and PB_0 (TIM1_CH2N), PB_0 will mirror PA_9 with flipped polarity. Note that when used in pairing mode, the duty cycle is locked to the non-N pin meaning you won't be able to alter the duty cycle of any N pins independantly and you must alter the duty cycle of the non-N pin to control it. Variable period works fine for these and they stay in sync.
- If you don't specify a fixed period in your config.txt, or if your LinuxCNC intialises this as zero, the default will become 200us
- Without a linuxCNC config controlling a set point variable, the PWM will automatically starts as soon as you press the eStop. You will need to configure LinuxCNC to stop and start on the conditions you want. For example when using it as a 0-10v Spindle control. HAL config can be found here: https://remora-docs.readthedocs.io/en/latest/software/hal-examples.html#pwm-to-0-10v-spindle-control-simple
- How many PWM pins available will be limited by your remora-eth-3.0 component, which defaults to 6. You can raise this limit by changing both remora-eth-3.h file and configuration.h file, but be careful that it fits within the data allocated for RX packets. You will need 1 variable for each fixed period PWM pin, or 2 for variable duty PWM.
- All PWM timers are either 16 or 32 bits wide depending on which TIMx is used. Either is more than enough for very fine control over duty cycle.
- Be very careful not to clash with other peripherals such as SPI or UART, this will result in undefined behaviour. 

PWM compatible pins for smaller F446xx target are:
| Pin   | Timer | Channel | Notes                    | Tested?     |
|-------|-------|---------|--------------------------|-------------|
| PA_8  | TIM1  | CH1     |                          | Working     |
| PA_9  | TIM1  | CH2     |                          | Working     |
| PA_10 | TIM1  | CH3     |                          | Working     |
| PA_11 | TIM1  | CH4     |                          | Working     |
| PB_0  | TIM1  | CH2N    | Inverted PA_9            | Working     |
| PB_1  | TIM1  | CH3N    | Inverted PA_10           | Working     |
| PB_6  | TIM4  | CH1     | Possible conflict SPI_CS | Not Tested  |
| PB_7  | TIM4  | CH2     |                          | Working     |
| PB_13 | TIM1  | CH1N    | Inverted PA_8            | Not Tested  |
| PB_14 | TIM1  | CH2N    | Inverted PA_9            | Not Tested  |
| PB_15 | TIM1  | CH3N    | Inverted PA_10           | Not Tested  |

Additional pins for larger F4 boards like the F446ZE
| Pin   | Timer | Channel | Notes                    | Tested?     |
|-------|-------|---------|--------------------------|-------------|
| PD_12 | TIM4  | CH1     |                          | Not Tested  |
| PD_13 | TIM4  | CH2     |                          | Not Tested  |
| PD_14 | TIM4  | CH3     |                          | Not Tested  |
| PD_15 | TIM4  | CH4     |                          | Not Tested  |
| PE_5  | TIM9  | CH1     |                          | Not Tested  |
| PE_6  | TIM9  | CH2     |                          | Not Tested  |
| PE_8  | TIM1  | CH1N    |                          | Not Tested  |
| PE_9  | TIM1  | CH1     |                          | Not Tested  |
| PE_10 | TIM1  | CH2N    |                          | Not Tested  |
| PE_11 | TIM1  | CH2     |                          | Not Tested  |
| PE_12 | TIM1  | CH3N    |                          | Not Tested  |
| PE_13 | TIM1  | CH3     |                          | Not Tested  |
| PE_14 | TIM1  | CH4     |                          | Not Tested  |
| PF_6  | TIM10 | CH1     |                          | Not Tested  |
| PF_7  | TIM11 | CH1     |                          | Not Tested  |
| PF_8  | TIM13 | CH1     |                          | Not Tested  |
| PF_9  | TIM14 | CH1     |                          | Not Tested  |
 
# Boards
- Nucleo F446RE: In development
- Nucleo F446ZE: In development

------------------------------------------

# Installation instructions
You may build the firmware from source using PlatformIO. On initial release, precompiled binaries will be made available in this repo enabling upload via an STLink. 

This firmware uses the Remora-eth-0.3.0 ethernet component avaialable in the LinuxCNC/Components folder.

Compile the component using halcompile
```
sudo halcompile --install remora-eth-3.0.c
```

Configs are loaded via tftpy, on release a config.txt file will be made available which you can upload via an upload_config.py script.
```
pip3 install tftpy # If not using virtualenv you may get an error about breaking system packages, use the --break-system-packages flag if needed
python3 upload_config.py NucleoF411RE-Config.txt
```

Refer to the remora documents for more information
https://remora-docs.readthedocs.io/en/latest/firmware/ethernet-config.html

Board will not start until ethernet connection is established. 

Credits to Scotta and Cakeslob and others that worked on Remora. Additional credit to Expatria Technologies and Terje IO. 

# Known issues
- See status for info first, other specific bugs to be recorded here as we go.
- Noticed that STMHal makes heavy use of lock objects, not sure if keeping generic HAL Handlers in each class is going to work long term. See the Hardware PWM HAL code for more info, they needed to be broken out as global objects so that the class can allocate shared resources. This issue may also creep up later with shared SPI and other handlers later on down the track