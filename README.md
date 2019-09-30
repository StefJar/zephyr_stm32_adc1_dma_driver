# STM32F4 async ADC driver with DMA transfer

Simple ZephyrOS device driver for the stm32f4 series setting. It sets up the ADC1 + DMA2 and connects it via dma streams. The driver manages the memory of the sample buffer. The samples can be accessed via API (`api->sb` and `api->sbN`). A bit dirty but good enough as demo.

The `init` function inits the driver. After, `convert` triggers the AD conversion. This function returns when the convertion is successfully done. The `close` function disables the ADC driver.

Note:

The driver is designed to generate the smallest amount of CPU load. Thats why a fixed buffer is used. The DMA stream/channel is set to circular. This saves the insturctions to setup the DMA memory at every transfer.

The `ADC1->DR` register is 32Bit. So we need 32Bit DMA transfers.

sources:
- stm32f412 datasheet[(RM0402)](https://www.st.com/content/ccc/resource/technical/document/reference_manual/group0/4f/7b/2b/bd/04/b3/49/25/DM00180369/files/DM00180369.pdf/jcr:content/translations/en.DM00180369.pdf)
- stm32HAL LL documentation [(UM1725)](https://www.st.com/content/ccc/resource/technical/document/user_manual/2f/71/ba/b8/75/54/47/cf/DM00105879.pdf/files/DM00105879.pdf/jcr:content/translations/en.DM00105879.pdf)
