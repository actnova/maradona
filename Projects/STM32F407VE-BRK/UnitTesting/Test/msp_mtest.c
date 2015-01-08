/*
 * msp_mtest.c
 *
 *  Created on: 2015Äê1ÔÂ8ÈÕ
 *      Author: ma
 */


// msp_create_uartex_handle



#include <mocha.h>
#include "msp.h"

extern int __mock_gpio_create_handle;
extern int __mock_gpio_destroy_handle;

static void setup(void** state) {

	MOCHA_ON(gpio_create_handle);
	MOCHA_ON(gpio_destroy_handle);
}

static void teardown(void** state) {

	MOCHA_OFF(gpio_create_handle);
	MOCHA_OFF(gpio_destroy_handle);
}

// UARTEX_HandleTypeDef* msp_create_uartex_handle(struct msp_factory* msp, const UARTEX_ConfigTypeDef* cfg);

void test_msp_create_uartex_handle_fail_first_gpio_create_handle(void** state) {

//	gpio_handle_t* gpio_create_handle(const gpio_config_t* conf, gpio_man_t *man)

	gpio_config_t gconf;
	gpio_man_t gman;
	UARTEX_ConfigTypeDef cfg;
	struct msp_factory msp;

	EXPECT(gpio_create_handle, conf, &gconf);
	EXPECT(gpio_create_handle, man, &gman);
	RETURN(gpio_create_handle, 0);
/**
 * 	if (msp == NULL || msp->dma_clk == NULL || msp->gpio_clk == NULL || msp->irq_registry == NULL ||
 * 	msp->create_dmaex_handle == NULL || cfg == NULL) return NULL;
 */
	msp.dma_clk = (DMA_ClockProviderTypeDef*)0xDEADBEEF;
	msp.gpio_clk = &gman;
	msp.irq_registry = (IRQ_HandleRegistryTypeDef*)0xDEADBEEF;
	/** cast integer to function pointer **/
	msp.create_dmaex_handle = (DMAEX_HandleTypeDef*	(*)(struct msp_factory * msp, const DMA_ConfigTypeDef * dmacfg, const IRQ_ConfigTypeDef * irqcfg)) 0xDEADBEEF;
	cfg.rxpin = &gconf;

	assert_true(0 == msp_create_uartex_handle(&msp, &cfg));
}

void test_msp_create_uartex_handle_fail_second_gpio_create_handle(void** state) {

//	gpio_handle_t* gpio_create_handle(const gpio_config_t* conf, gpio_man_t *man)

	gpio_config_t grxconf, gtxconf;
	gpio_man_t gman;
	UARTEX_ConfigTypeDef cfg;
	struct msp_factory msp;

/**
 * 	if (msp == NULL || msp->dma_clk == NULL || msp->gpio_clk == NULL || msp->irq_registry == NULL ||
 * 	msp->create_dmaex_handle == NULL || cfg == NULL) return NULL;
 */
	msp.dma_clk = (DMA_ClockProviderTypeDef*)0xDEADBEEF;
	msp.gpio_clk = &gman;
	msp.irq_registry = (IRQ_HandleRegistryTypeDef*)0xDEADBEEF;
	/** cast integer to function pointer **/
	msp.create_dmaex_handle = (DMAEX_HandleTypeDef*	(*)(struct msp_factory * msp, const DMA_ConfigTypeDef * dmacfg, const IRQ_ConfigTypeDef * irqcfg)) 0xDEADBEEF;
	cfg.rxpin = &grxconf;
	cfg.txpin = &gtxconf;

	// first call
	EXPECT(gpio_create_handle, conf, &grxconf);
	EXPECT(gpio_create_handle, man, &gman);
	RETURN(gpio_create_handle, 0xFEEDBEEF);

	// second call
	EXPECT(gpio_create_handle, conf, &gtxconf);
	EXPECT(gpio_create_handle, man, &gman);
	RETURN(gpio_create_handle, 0);

	// no return, see mock
	EXPECT(gpio_destroy_handle, h, 0xFEEDBEEF);

	assert_true(0 == msp_create_uartex_handle(&msp, &cfg));
}

int run_tests_msp(void) {

	const UnitTest tests[] = {

		unit_test_setup_teardown(test_msp_create_uartex_handle_fail_first_gpio_create_handle, setup, teardown),
		unit_test_setup_teardown(test_msp_create_uartex_handle_fail_second_gpio_create_handle, setup, teardown),
	};

	return run_tests(tests);
}
