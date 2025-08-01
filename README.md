# Remora-STM32F4xx-PIO
Port or Remora for STM32F4xx family, driven by latest Remora-Core abstraction layer.

# Todos / Status
- JSON config loading - in progress. Todos:
    - Set up NOINIT in the linker script so that config is not overwritten
    - Come back to the code that loads using TFTPY after you have set up Ethernet control and a default config
- Wiznet control code - in progress. Todos:
    - Testing of Wiznet handling code
    - Testing of Lwip handling code
    - Connect up the DMA transfers between Phy, UDP and CommsInterface layers. 
    - Scope out the Wiznet module INT line, can this be used for IRQ for recieving control packets?    
    - Test controls to / from LinuxCNC
    - Set up config loading via TFTP and hook into the JSON config loading
- Uart3 supported for console logging. Some support for others, platformIO config to be tested. Also need to create some abstractions around this.
- Stepgen, blink, digital IO, analog Ins, and PWM to be ported
- Software PWM yet to be ported.
- Encoder modules yet to be ported.
- Linker scripts to be built for range of F4xx builds
- Turns out, refactoring our CONTROL_METHOD build flags was due to a silly error where I didn't inherit the right build flags. Check with the guys and see what they think should be the standard across multiple builds.
- ADC - move MSP init code into class
- Clear up conflict in DMA IRQs between SPI and Ethernet comms modules, just some simple include guarding will suffice.
- Test out differnet clock config code, what's different between the boards?
- Test code to clear up:
    - Alternate clock config for testing.
    - Extra DMA and SPI callback functions



# Wiznet W5500 connection
- PA_5: SCK
- PA_6: MISO
- PA_7: MOSI
- WIZ_RST and SPI_CS are configured in Platformio.ini. 
- SPI_INT is not yet used by this firmware

# UART connections
- PD_9: UART_TX
- PD_8: UART_RX - Firmware doesn't make use of this, but left for completeness and future expansion

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
Building the firmware from source using PlatformIO. On initial release, precompiled binaries will be made available in this repo enabling upload via an STLink. 

This firmware uses the Remora-eth-0.3.0 ethernet component avaialable on the NVEM Remora port found here: https://github.com/scottalford75/Remora-RT1052-cpp/tree/main/LinuxCNC/components/Remora-eth

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