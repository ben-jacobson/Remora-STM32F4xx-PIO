# Remora-Core-F4-Port
Remora-Core-STM32F4xx

# Todos
- Get JSON config loading working:
    - Is there any way of loading a default config like in previous?
    - Is there memory allocated?
    - Set up NOINIT in the linker script so that config is not overwritten
    - Come back to the code that loads using TFTPY after you have set up Ethernet control and a default config
- Get Ethernet control working.
    - Plug in the data packet into existing DMA code
    - Come back to config uploading
    - Test that we haven't re-created bug found in other version where segment start address was incorrect
- Test SPI configuration, architecture is quite different from reference code base which uses SDMMC, ours is SDIO and will require some testing and refactoring to make it work