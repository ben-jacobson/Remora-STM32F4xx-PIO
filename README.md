# Remora-STM32F4xx-PIO
Port or Remora for STM32F4xx family, using new Remora-Core abstraction.

# Todos
- Get JSON config loading working:
    - Set up NOINIT in the linker script so that config is not overwritten
    - Come back to the code that loads using TFTPY after you have set up Ethernet control and a default config
- Get Ethernet control working.
    - Scope the Wiznet module INT line, can this be used for IRQ for recieving control packets?    
    - Set up CommsInterface for Ethernet control, setting up a SPI interface and DMA for efficient data copy 
    - Test control
    - Test config loading and hook into JSON config loading