# Remora-STM32F4xx-PIO
Port or Remora for STM32F4xx family, ported to use the latest Remora-Core abstraction layer.

# Todos / Status
- Network control code - in progress:
    - Current status:
        - Wiznet and LWIP integrated
        - Connects to the network
        - Using the Remora-Eth-3.0 LinuxCNC component, able to switch the PRU in and out of eStop modes.
    - Todos:
        - Set up FTP client for JSON config loading
        - See if we can make use of the Wiznet INT output somehow
        - Work with the team on an idea for Remora to trigger an interrupt 
        - Test for checking around lost packets
- JSON config loading - in progress:
    - Current status:
        - Able to detect empty config and load default blinky. Blinky module working
    - Todos:
        - Set up the linker script to allocate memory for the config
        - Once FTP is set up, hook up the functions for loading config into memory and resetting PRU
        - Once config loading is complete, upload our test config and run some tests for stepgen, digital in and digital out
- SPI Comms - in progress:
    - Current status:
        - Ported in and builds, not yet tested
    - Todos:
        - Find a LinuxCNC image for RPI3 as this is all we have on hand. 
        - Full testing required on SPIComms ported in from H7
        - Some refactoring to use new introduced interface features and hal_utils. 
        - Refactoring of Mbed code used for handling DMA interrupts. See if we can replace with a set of idiomatic STM HAL DMA handlers
- Uart3 support - in progress: 
    - Current status:
        - Basic set up for console logging on a select peripheral set in PlatformIO.ini
    - Todos:
        - Refactor so that this is handled by a class. Perhaps use the comms interface? 
- Modules - in progress:
    - Blinky - working
    - Stepgen, digital IO, Analog Ins - to be tested
    - Hardware PWM yet to be ported in from deprecated F4 code base
    - Encoder modules yet to be ported.
    - Linker scripts to be built for a range of F4xx builds
- Adhoc todos: 
    - ADC - move MSP init code into class
    - Clear up conflict in DMA IRQs between SPI and Ethernet comms modules, just some simple include guarding will suffice.
    - Test out differnet clock config code, what's different between the boards?
    - Look at IRQHandle and move the DMAStreams over to unused channel/stream combos for SPIComms on F4

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
- Nucleo F446RE : In development
- Nucleo F446ZE : In development

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