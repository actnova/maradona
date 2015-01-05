/**
  ******************************************************************************
  * File Name          : gpio.c
  * Date               : 11/06/2014 12:17:27
  * Description        : This file provides code for the configuration
  *                      of all used GPIO pins.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2014 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include <stdbool.h>
#include <string.h>

#include "gpio.h"
#include "errno_ex.h"


gpio_clock_t GPIO_ClockProvider = {{0}};

void	GPIOEX_HAL_Init(gpio_handle_t* gpioex)
{
	gpio_clk_get(gpioex->clk, gpioex->instance, gpioex->init.Pin);
	HAL_GPIO_Init(gpioex->instance, &gpioex->init);
	gpioex->state = GPIOEX_STATE_SET;
}

void 	GPIOEX_HAL_DeInit(gpio_handle_t* gpioex)
{
	HAL_GPIO_DeInit(gpioex->instance, gpioex->init.Pin);
	gpio_clk_put(gpioex->clk, gpioex->instance, gpioex->init.Pin);
	gpioex->state = GPIOEX_STATE_RESET;

}

void gpio_clk_get(gpio_clock_t* clk, GPIO_TypeDef* gpiox, uint32_t Pin) 
{
	int index;
	uint16_t prev, curr;
	
	index = _gpio_reg2index(gpiox);

	if (index < 0)
		return;
	
	prev = clk->bits[index];
	curr = (clk->bits[index] |= Pin);		/**** need new case, get twice, put once, clock on ****/

	if (prev == 0 && curr != 0)
	{
		_gpio_clk_enable(index);
	}
}


void gpio_clk_put(gpio_clock_t* clk, GPIO_TypeDef* gpiox, uint32_t Pin)
{
	int index;
	uint16_t prev, curr;	
	
	index = _gpio_reg2index(gpiox);

	if (index < 0)
		return;
	
	prev = clk->bits[index];
	curr = (clk->bits[index] &= ~Pin);

	if (prev != 0 && curr == 0)
	{
		_gpio_clk_disable(index);
	}
}

int gpio_clk_status(gpio_clock_t* clk, GPIO_TypeDef* gpiox, uint32_t Pin)
{
	int index;
	
	index = _gpio_reg2index(gpiox);

	if (index < 0)
		return -1;

	
	return (clk->bits[index] & Pin) ? 1 : 0;
}

int GPIOEX_Init(gpio_handle_t* ge, GPIO_TypeDef* gpiox, const GPIO_InitTypeDef* init, gpio_clock_t* clk)
{
	if (ge == NULL || gpiox == NULL || init == NULL || clk == NULL)
		return -EINVAL;

	memset(ge, 0, sizeof(gpio_handle_t));

	ge->instance = gpiox;
	memmove(&ge->init, init, sizeof(GPIO_InitTypeDef));
	ge->clk = clk;
	ge->state = GPIOEX_STATE_RESET;

	return 0;
}

int GPIOEX_InitByConfig(gpio_handle_t* ge, const gpio_config_t* config, gpio_clock_t* clk)
{
	return GPIOEX_Init(ge, config->instance, &config->init, clk);
}

gpio_handle_t* alloc_gpio_handle(const gpio_config_t* conf, gpio_clock_t *clk) {

	gpio_handle_t* h;

	if (conf == NULL || clk == NULL)
		return NULL;

	h = (gpio_handle_t*)malloc(sizeof(gpio_handle_t));
	if (h == NULL) return NULL;

	h->instance = conf->instance;
	memmove(&h->init, &conf->init, sizeof(GPIO_InitTypeDef));
	h->clk = clk;
	return h;
}

/************************ Defaults ********************************************/

const gpio_config_t	PC6_As_Uart6Tx_DefaultConfig =
{
	.instance = GPIOC,
	.init = 
	{
		.Pin = GPIO_PIN_6,
		.Mode = GPIO_MODE_AF_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_LOW,
		.Alternate = GPIO_AF8_USART6,					
	},
};


const gpio_handle_t	PC6_As_Uart6Tx_Default =
{
	.instance = GPIOC,
	.init =
	{
		.Pin = GPIO_PIN_6,
		.Mode = GPIO_MODE_AF_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_LOW,
		.Alternate = GPIO_AF8_USART6,			
	},
	.clk = &GPIO_ClockProvider,			// don't forget this! bug!
};

const gpio_config_t	PD6_As_Uart2Rx_DefaultConfig = 
{
	.instance = GPIOD,
	.init =
	{
		.Pin = GPIO_PIN_6,
		.Mode = GPIO_MODE_AF_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_LOW,
		.Alternate = GPIO_AF7_USART2,			
	},
};

const gpio_handle_t	PD6_As_Uart2Rx_Default = 
{
	.instance = GPIOD,
	.init =
	{
		.Pin = GPIO_PIN_6,
		.Mode = GPIO_MODE_AF_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_LOW,
		.Alternate = GPIO_AF7_USART2,			
	},
	.clk = &GPIO_ClockProvider,
};

const gpio_config_t	PD5_As_Uart2Tx_DefaultConfig =
{
	.instance = GPIOD,
	.init =
	{
		.Pin = GPIO_PIN_5,
		.Mode = GPIO_MODE_AF_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_LOW,
		.Alternate = GPIO_AF7_USART2,			
	},
};

const gpio_handle_t	PD5_As_Uart2Tx_Default =
{
	.instance = GPIOD,
	.init =
	{
		.Pin = GPIO_PIN_5,
		.Mode = GPIO_MODE_AF_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_LOW,
		.Alternate = GPIO_AF7_USART2,			
	},
	.clk = &GPIO_ClockProvider,		
};

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
