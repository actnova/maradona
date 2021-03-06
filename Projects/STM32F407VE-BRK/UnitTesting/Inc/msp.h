#ifndef MSP_H
#define MSP_H

/** not sure if this module should be named as msp, or board, or hal. **/
#include "stm32f4xx_hal.h"

#include "errno_ex.h"
#include "board_config.h"
#include "dma.h"
#include "gpio.h"
#include "irq.h"
#include "usart.h"


struct msp_factory
{
	const Board_ConfigTypeDef*		board_config;
	
	gpio_man_t*						gpio_clk;
	DMA_ClockProviderTypeDef*		dma_clk;
	IRQ_HandleRegistryTypeDef* 		irq_registry;
	
	
	/////////////////////////////////////////////////////////////////////////////
	// these are member methods
	UARTEX_HandleTypeDef* (*create_uartex_handle_by_port)(struct msp_factory * msp, int num);
	UARTEX_HandleTypeDef* (*create_uartex_handle)(struct msp_factory * msp, const UARTEX_ConfigTypeDef * cfg);	
	void (*destroy_uartex_handle)(struct msp_factory * msp, UARTEX_HandleTypeDef* );

	DMAEX_HandleTypeDef*	(*create_dmaex_handle)(struct msp_factory * msp, const DMA_ConfigTypeDef * dmacfg, const IRQ_ConfigTypeDef * irqcfg);
	void (*destroy_dmaex_handle)(struct msp_factory * msp, DMAEX_HandleTypeDef* handle);	
	
	/////////////////////////////////////////////////////////////////////////////
	// required functions
	// int (*gpioex_init_by_config)(gpio_handle_t* h, const gpio_config_t* conf, gpio_man_t* man);
	int	(*irq_handle_init_by_config)(IRQ_HandleTypeDef* h, const IRQ_ConfigTypeDef* config, IRQ_HandleRegistryTypeDef* registry);
	int	(*uartex_handle_init_by_config)(UARTEX_HandleTypeDef* h, const UART_ConfigTypeDef	*config, gpio_handle_t	*rxpin, gpio_handle_t *txpin, 
		DMAEX_HandleTypeDef *hdmaex_rx, DMAEX_HandleTypeDef *hdmaex_tx, IRQ_HandleTypeDef *hirq, const struct UARTEX_Operations		*ops);
	
	/////////////////////////////////////////////////////////////////////////////
	// test data
	void * testdata;
};

///////////////////////////////////////////////////////////////////////////////
// factory methods for uartex_handle
UARTEX_HandleTypeDef* msp_create_uartex_handle(struct msp_factory * msp, const UARTEX_ConfigTypeDef * cfg);
void msp_destroy_uartex_handle(struct msp_factory * msp, UARTEX_HandleTypeDef* );
UARTEX_HandleTypeDef* msp_create_uartex_handle_by_port(struct msp_factory * msp, int port);

///////////////////////////////////////////////////////////////////////////////
// factory methods for dmaex_handle
DMAEX_HandleTypeDef*	msp_create_dmaex_handle(struct msp_factory * msp, const DMA_ConfigTypeDef * dmacfg, const IRQ_ConfigTypeDef * irqcfg);
void msp_destroy_dmaex_handle(struct msp_factory * msp, DMAEX_HandleTypeDef* handle);

///////////////////////////////////////////////////////////////////////////////
// globals
extern struct msp_factory MSP;

#endif

