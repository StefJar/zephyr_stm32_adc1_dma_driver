/*
 * adc_stm32.h
 *
 *  Created on: 24 Sep 2019
 *      Author: stefan jaritz
 */

#ifndef DRV_ADC_DMA_H_
#define DRV_ADC_DMA_H_

#include <device.h>
#include <stdbool.h>

#define ADC_DMA_NAME "ADC1DMA"

typedef enum adc_dma_errors {
	adc_dma_error_success = 0,
} adc_dma_error_t;

typedef enum adc_dma_channels {
	adc_dma_channel_11 = 0,
	adc_dma_channel_13 = 1,
	adc_dma_channel_vref = 2,

	adc_dma_channel_N
}adc_dma_channel_t;

typedef u32_t adc_dma_samples_t [adc_dma_channel_N];

typedef struct adc_dma_api_s {
	adc_dma_error_t (* init) (void);
	void (* close) (void);
	adc_dma_error_t (* convert)(void);
	const u32_t * sb; //!< pointer to sample buffer
	const unsigned int sbN; //!< amount of samples in the sample buffer
} adc_dma_api_t;



#endif /* DRV_ADC_DMA_H_ */
