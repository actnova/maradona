/*
 * gpio_stub.c
 *
 *  Created on: 2015Äê1ÔÂ5ÈÕ
 *      Author: ma
 */

#include "gpio.h"
#include "errno_ex.h"

#define GPIO_PORT_NUM	(8)

const static GPIO_TypeDef* gpio_regs[GPIO_PORT_NUM] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH};

int _gpio_reg2index(GPIO_TypeDef* gpiox) {

	int i;
	for (i = 0; i < GPIO_PORT_NUM; i++) {
		if (gpiox == gpio_regs[i]) {
			return i;
		}
	}

	return -1;
}

int _gpio_clk_enable(int index) {

	if (index < 0 || index > GPIO_PORT_NUM - 1)
		return -1;

	switch (index)
	{
		case 0:
			__GPIOA_CLK_ENABLE();
			break;
		case 1:
			__GPIOB_CLK_ENABLE();
			break;
		case 2:
			__GPIOC_CLK_ENABLE();
			break;
		case 3:
			__GPIOD_CLK_ENABLE();
			break;
		case 4:
			__GPIOE_CLK_ENABLE();
			break;
		case 5:
			__GPIOF_CLK_ENABLE();
			break;
		case 6:
			__GPIOG_CLK_ENABLE();
			break;
		case 7:
			__GPIOH_CLK_ENABLE();
			break;
		default:
			return -1;	// invalid index
	}

	return 0;
}

int _gpio_clk_disable(int index) {

	if (index < 0 || index > GPIO_PORT_NUM - 1)
		return -1;

	switch (index)
	{
		case 0:
			__GPIOA_CLK_DISABLE();
			break;
		case 1:
			__GPIOB_CLK_DISABLE();
			break;
		case 2:
			__GPIOC_CLK_DISABLE();
			break;
		case 3:
			__GPIOD_CLK_DISABLE();
			break;
		case 4:
			__GPIOE_CLK_DISABLE();
			break;
		case 5:
			__GPIOF_CLK_DISABLE();
			break;
		case 6:
			__GPIOG_CLK_DISABLE();
			break;
		case 7:
			__GPIOH_CLK_DISABLE();
			break;
		default:
			break;
	}

	return 0;
}
