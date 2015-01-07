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

gpio_man_t GPIO_Manager = {{0}};

void GPIOEX_HAL_Init(gpio_handle_t* gpio)
{
	gpio_request_pin(gpio->man, gpio->instance, gpio->init.Pin);
	HAL_GPIO_Init(gpio->instance, &gpio->init);
	gpio->state = GPIOEX_STATE_SET;
}

void GPIOEX_HAL_DeInit(gpio_handle_t* gpio)
{
	HAL_GPIO_DeInit(gpio->instance, gpio->init.Pin);
	gpio_release_pin(gpio->man, gpio->instance, gpio->init.Pin);
	gpio->state = GPIOEX_STATE_RESET;
}

int gpio_man_init(gpio_man_t* man) {

	if (man) {
		memset(man, 0, sizeof(gpio_man_t));
		return 0;
	}

	return -1;
}

/*
 * This function check if given parameters are valid.
 *
 * return gpio port index for convenience
 */
static int gpio_check_params(gpio_man_t* man, GPIO_TypeDef* gpio, uint32_t pin) {

	int index, i, count;

	if (man == NULL)
		return -1;

	index = _gpio_reg2index(gpio);
	if (index < 0)
		return -1;

	count = 0;
	for (i = 0; i < 32; i++) {
		if (pin & (1 << i)) {
			count++;
		}
	}

	if (count != 1)		// only one bit is allowed to set.
		return -1;

	return index;
}

static int _gpio_in_use(gpio_man_t* man, int index , uint32_t pin)
{
	return (man->bits[index] & pin) ? 1 : 0;
}

int gpio_request_pin(gpio_man_t* man, GPIO_TypeDef* gpio, uint32_t pin)
{
	int index;
	uint16_t prev, curr;

	index = gpio_check_params(man, gpio, pin);
	if (index < 0)
		return index;

	if (_gpio_in_use(man, index, pin))
		return -1;

	prev = man->bits[index];
	curr = (man->bits[index] |= pin);

	if (prev == 0 && curr != 0)
	{
		_gpio_clk_enable(index);
	}

	return 0;
}

int gpio_release_pin(gpio_man_t* man, GPIO_TypeDef* gpio, uint32_t pin)
{
	int index;
	uint16_t prev, curr;
	
	index = gpio_check_params(man, gpio, pin);
	if (index < 0)
		return index;

	if (!_gpio_in_use(man, index, pin))
		return -1;
	
	prev = man->bits[index];
	curr = (man->bits[index] &= ~pin);

	if (prev != 0 && curr == 0)
	{
		_gpio_clk_disable(index);
	}

	return 0;
}

int gpio_pin_in_use(gpio_man_t* man, GPIO_TypeDef* gpio, uint32_t pin)
{
	int index;

	index = gpio_check_params(man, gpio, pin);
	if (index < 0)
		return index;
	
	return _gpio_in_use(man, index, pin);
}

int GPIOEX_Init(gpio_handle_t* ge, GPIO_TypeDef* gpiox, const GPIO_InitTypeDef* init, gpio_man_t* clk)
{
	if (ge == NULL || gpiox == NULL || init == NULL || clk == NULL)
		return -EINVAL;

	memset(ge, 0, sizeof(gpio_handle_t));

	ge->instance = gpiox;
	memmove(&ge->init, init, sizeof(GPIO_InitTypeDef));
	ge->man = clk;
	ge->state = GPIOEX_STATE_RESET;

	return 0;
}

int GPIOEX_InitByConfig(gpio_handle_t* ge, const gpio_config_t* config, gpio_man_t* clk)
{
	return GPIOEX_Init(ge, config->instance, &config->init, clk);
}

gpio_handle_t* gpio_create_handle(const gpio_config_t* conf, gpio_man_t *man) {

	gpio_handle_t* h;

	if (conf == NULL || man == NULL)
		return NULL;

	h = (gpio_handle_t*)malloc(sizeof(gpio_handle_t));
	if (h == NULL) return NULL;
	memset(h, 0, sizeof(gpio_handle_t));

	h->instance = conf->instance;
	memmove(&h->init, &conf->init, sizeof(GPIO_InitTypeDef));
	h->man = man;
	h->state = GPIOEX_STATE_RESET;
	return h;
}

int gpio_destroy_handle(gpio_handle_t* h) {

	if (h) {
		free(h);
		return 0;
	}

	return -1;
}

/*
 * utility function for test hal registers
 */
int gpio_clock_is_enabled(GPIO_TypeDef* gpio)
{
	if (GPIOA == gpio)
		return (RCC->AHB1ENR & (RCC_AHB1ENR_GPIOAEN)) ? 1 : 0;
	else if (GPIOB == gpio)
		return (RCC->AHB1ENR & (RCC_AHB1ENR_GPIOBEN)) ? 1 : 0;
	else if (GPIOC == gpio)
		return (RCC->AHB1ENR & (RCC_AHB1ENR_GPIOCEN)) ? 1 : 0;
	else if (GPIOD == gpio)
		return (RCC->AHB1ENR & (RCC_AHB1ENR_GPIODEN)) ? 1 : 0;
	else if (GPIOE == gpio)
		return (RCC->AHB1ENR & (RCC_AHB1ENR_GPIOEEN)) ? 1 : 0;
	else if (GPIOF == gpio)
		return (RCC->AHB1ENR & (RCC_AHB1ENR_GPIOFEN)) ? 1 : 0;
	else if (GPIOG == gpio)
		return (RCC->AHB1ENR & (RCC_AHB1ENR_GPIOGEN)) ? 1 : 0;
	else if (GPIOH == gpio)
		return (RCC->AHB1ENR & (RCC_AHB1ENR_GPIOHEN)) ? 1 : 0;
	else
		return -1;
}

bool gpio_mode_set(GPIO_TypeDef* GPIOx, GPIO_InitTypeDef* init)
{
	uint32_t position = 0, moder, afr;

	while (!(init->Pin & ((uint32_t)1 << position)))
	{
		position++;
	}

	moder = GPIOx->MODER;
	moder = moder >> (position * 2);
	moder = moder & (uint32_t)3;

	if (moder != (init->Mode & (uint32_t)3))
		return false;

	// af = ((uint32_t)(init->Alternate) << (((uint32_t)position & (uint32_t)0x07) * 4));
	afr = GPIOx->AFR[position >> 3];
	afr = afr >> (((uint32_t)position & (uint32_t)0x07) * 4);
	afr = afr & ((uint32_t)0xF);

	if (afr != init->Alternate)
		return false;

	return true;

// GPIOx->MODER &= ~(GPIO_MODER_MODER0 << (position * 2));
// GPIOx->MODER |= ((GPIO_Init->Mode & GPIO_MODE) << (position * 2));

///* Configure Alternate function mapped with the current IO */
//temp = ((uint32_t)(GPIO_Init->Alternate) << (((uint32_t)position & (uint32_t)0x07) * 4)) ;
//GPIOx->AFR[position >> 3] &= ~((uint32_t)0xF << ((uint32_t)(position & (uint32_t)0x07) * 4)) ;
//GPIOx->AFR[position >> 3] |= temp;
}

bool gpio_mode_reset(GPIO_TypeDef* GPIOx, GPIO_InitTypeDef* init)
{
	uint32_t position = 0, moder, afr;

	while (!(init->Pin & ((uint32_t)1 << position)))
	{
		position++;
	}

	moder = GPIOx->MODER;
	moder = moder >> (position * 2);
	moder = moder & (uint32_t)3;

	if (moder != 0)
		return false;

	afr = GPIOx->AFR[position >> 3];
	afr = afr >> (((uint32_t)position & (uint32_t)0x07) * 4);
	afr = afr & ((uint32_t)0xF);

	if (afr != 0)
		return false;

	return true;

///*------------------------- GPIO Mode Configuration --------------------*/
///* Configure IO Direction in Input Floating Mode */
//GPIOx->MODER &= ~(GPIO_MODER_MODER0 << (position * 2));

///* Configure the default Alternate Function in current IO */
//GPIOx->AFR[position >> 3] &= ~((uint32_t)0xF << ((uint32_t)(position & (uint32_t)0x07) * 4)) ;
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
	.man = &GPIO_Manager,			// don't forget this! bug!
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
	.man = &GPIO_Manager,
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
	.man = &GPIO_Manager,
};

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
