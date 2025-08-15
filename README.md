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

# Software PWM
Software PWM is not yet supported and may not ever be. Please use Hardware PWM for the time being, the amount of flexibility offered by the hardware may override the need for Software PWM. 

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