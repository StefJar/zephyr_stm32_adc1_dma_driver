/*
 * adc_dma.c
 *
 *  Created on: 24 Sep 2019
 *      Author: stefan jaritz
 */

#include "adc_dma.h"
#include <zephyr.h>
#include <stm32f4xx_ll_dma.h>
#include <stm32f4xx_ll_adc.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(ADC1DMA, 4);

#define ADC1_DMA DMA2
#define ADC1_DMA_CHANNEL LL_DMA_CHANNEL_0
#define ADC1_DMA_STREAM LL_DMA_STREAM_0
#define ADC1_DMA_INTR_PRIO 3

typedef struct adc1_dma_s {
	struct k_sem adcDone;
	adc_dma_samples_t sb;
} adc1_dma_t;

static adc1_dma_t adc1_dma;

// adc dma isr handler
ISR_DIRECT_DECLARE(DMA2_Stream0_IRQHandler) {
	ISR_DIRECT_HEADER();
	// Check transfer-complete interrupt
	if (LL_DMA_IsEnabledIT_TC(ADC1_DMA, ADC1_DMA_STREAM) && LL_DMA_IsActiveFlag_TC0(ADC1_DMA)) {
		LL_DMA_ClearFlag_TC0(ADC1_DMA);             // Clear half-transfer complete flag
		// indicate that we are done
		k_sem_give(&adc1_dma.adcDone);
	}
	ISR_DIRECT_FOOTER(1);
	ISR_DIRECT_PM(); // PM done after servicing interrupt for best latency
	return 1; // We should check if scheduling decision should be made
}

// adc isr handler
ISR_DIRECT_DECLARE(ADC1_IRQHandler) {
	ISR_DIRECT_HEADER();
	LOG_ERR("wrong interrupt");
	//k_sem_give(&adc.done);

	ISR_DIRECT_FOOTER(1);
	ISR_DIRECT_PM(); // PM done after servicing interrupt for best latency
	return 1; // We should check if scheduling decision should be made
}

static adc_dma_error_t adc1_dma_init (void) {
	LL_ADC_InitTypeDef ADC_InitStruct = {0};
	LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};
	LL_ADC_CommonInitTypeDef ADC_CommonInitStruct = {0};

	LOG_DBG("init");

	// ADC1 dma init
	LL_DMA_SetChannelSelection(ADC1_DMA,ADC1_DMA_STREAM, ADC1_DMA_CHANNEL);
	LL_DMA_SetDataTransferDirection(ADC1_DMA,ADC1_DMA_STREAM, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
	LL_DMA_SetStreamPriorityLevel(ADC1_DMA,ADC1_DMA_STREAM, LL_DMA_PRIORITY_LOW);
	LL_DMA_SetMode(ADC1_DMA,ADC1_DMA_STREAM, LL_DMA_MODE_CIRCULAR);
	LL_DMA_SetPeriphIncMode(ADC1_DMA,ADC1_DMA_STREAM, LL_DMA_PERIPH_NOINCREMENT);
	LL_DMA_SetMemoryIncMode(ADC1_DMA,ADC1_DMA_STREAM, LL_DMA_MEMORY_INCREMENT);
	LL_DMA_SetPeriphSize(ADC1_DMA,ADC1_DMA_STREAM, LL_DMA_PDATAALIGN_WORD);
	LL_DMA_SetMemorySize(ADC1_DMA,ADC1_DMA_STREAM, LL_DMA_MDATAALIGN_WORD);
	LL_DMA_DisableFifoMode(ADC1_DMA,ADC1_DMA_STREAM);

	LL_DMA_SetPeriphAddress(ADC1_DMA, ADC1_DMA_STREAM, (u32_t)&ADC1->DR);
	LL_DMA_SetMemoryAddress(ADC1_DMA, ADC1_DMA_STREAM, (u32_t)adc1_dma.sb);
	LL_DMA_SetDataLength(ADC1_DMA, ADC1_DMA_STREAM, adc_dma_channel_N);

	// enable transfer complete interrupt
	LL_DMA_EnableIT_TC(ADC1_DMA, ADC1_DMA_STREAM);

	// adc init
	ADC_InitStruct.Resolution = LL_ADC_RESOLUTION_12B;
	ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
	ADC_InitStruct.SequencersScanMode = LL_ADC_SEQ_SCAN_ENABLE;
	LL_ADC_Init(ADC1, &ADC_InitStruct);
	ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
	ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_ENABLE_3RANKS;
	ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
	ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_CONTINUOUS;
	ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_UNLIMITED;

	LL_ADC_REG_Init(ADC1, &ADC_REG_InitStruct);
	LL_ADC_REG_SetFlagEndOfConversion(ADC1, LL_ADC_REG_FLAG_EOC_UNITARY_CONV);
	LL_ADC_DisableIT_EOCS(ADC1);
	ADC_CommonInitStruct.CommonClock = LL_ADC_CLOCK_SYNC_PCLK_DIV8;
	LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(ADC1), &ADC_CommonInitStruct);

	LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_11);
	LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_11, LL_ADC_SAMPLINGTIME_480CYCLES);

	LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_2, LL_ADC_CHANNEL_13);
	LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_13, LL_ADC_SAMPLINGTIME_480CYCLES);

	LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_3, LL_ADC_CHANNEL_VREFINT);
	LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_VREFINT, LL_ADC_SAMPLINGTIME_480CYCLES);
	LL_ADC_SetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_PATH_INTERNAL_VREFINT);

	// LL_ADC_EnableIT_EOCS(ADC1);

	LL_DMA_EnableStream(ADC1_DMA, ADC1_DMA_STREAM);
	return adc_dma_error_success;
}

static void adc1_dma_close (void) {
	if (0 == LL_ADC_IsEnabled(ADC1)) {
		LL_ADC_Disable(ADC1);
	}
	LL_DMA_DisableStream(ADC1_DMA, ADC1_DMA_STREAM);
}

static adc_dma_error_t adc1_dma_convert (void) {

	if (0 == LL_ADC_IsEnabled(ADC1)) {
		LL_ADC_Enable(ADC1);
		k_yield();
	}

	LL_ADC_REG_StartConversionSWStart(ADC1);
	k_sem_take(&adc1_dma.adcDone, K_FOREVER);
	LL_ADC_Disable(ADC1);
	return adc_dma_error_success;
}

static int adc1_dma_initilize(struct device *dev) {
	(void) dev;

	LOG_INF("initilize");

	memset(&adc1_dma, 0, sizeof(adc1_dma));
	k_sem_init(&adc1_dma.adcDone, 0, 1);

	// enable dma2 controller
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);
	// adc Peripheral clock enable
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC1);

	// connect intr
	IRQ_DIRECT_CONNECT(DMA2_Stream0_IRQn, ADC1_DMA_INTR_PRIO, DMA2_Stream0_IRQHandler, 0);
	irq_enable(DMA2_Stream0_IRQn);
	IRQ_DIRECT_CONNECT(ADC_IRQn, ADC1_DMA_INTR_PRIO, ADC1_IRQHandler, 0);
	irq_enable(ADC_IRQn);

	return 0;
}

static const adc_dma_api_t adc_dma_API = {
	.init = adc1_dma_init,
	.close = adc1_dma_close,
	.convert = adc1_dma_convert,
	.sb = adc1_dma.sb,
	.sbN = adc_dma_channel_N,
};

DEVICE_AND_API_INIT(adc1dma, ADC_DMA_NAME, &adc1_dma_initilize,
	NULL, NULL,
	POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
	&adc_dma_API
	);
